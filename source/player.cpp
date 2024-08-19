
static cardinal DeterminePlayerDirection(entity* Player, v2 Movement)
{
	// If no direction is dominant, bias towards keeping the player's current direction
	cardinal Result = Player->FacingCardinal;

	u8 DirFlags = DirectionsFromVector(Movement);

	v2 AbsMovement = v2 { Abs(Movement.X), Abs(Movement.Y) };
	if (AbsMovement.X > AbsMovement.Y)
	{
		if (DirFlags & Dir_Right)
		{
			Result = Cardinal_Right;
		}
		else
		{
			Assert(DirFlags & Dir_Left);
			Result = Cardinal_Left;
		}
	}
	else if (AbsMovement.X < AbsMovement.Y)
	{
		if (DirFlags & Dir_Down)
		{
			Result = Cardinal_Down;
		}
		else
		{
			Assert(DirFlags & Dir_Up);
			Result = Cardinal_Up;
		}
	}
	else
	{
		direction PlayerDir = CardinalToDirection(Player->FacingCardinal);
		if (!(DirFlags & PlayerDir))
		{
			// Corner case: if no direction is dominant, but player is moving in a direction he isn't
			// facing, we have to pick one somewhat arbitrarily
			if (DirFlags & Dir_Down)
			{
				Result = Cardinal_Down;
			}
			else if (DirFlags & Dir_Up)
			{
				Result = Cardinal_Up;
			}
			else if (DirFlags & Dir_Right)
			{
				Result = Cardinal_Right;
			}
			else if (DirFlags & Dir_Left)
			{
				Result = Cardinal_Left;
			}
		}
	}
	return Result;
}

enum collision_type : u8
{
	ColType_None,
	ColType_OutOfBounds,
	ColType_Wall,
	ColType_Npc,
	ColType_ScriptTile,
	ColType_Warp
};

struct collision_result
{
	collision_type Type;
	union
	{
		struct
		{
			u16 TileX;
			u16 TileY;
		};
		entity* OtherEntity;
		const warp_tile* TouchedWarp;
		const script_tile* ScriptTile;
	};
};

static collision_result WillCollide(entity* Player, v2 NewPos)
{
	collision_result Result = {};

	// We check collision against an 8x8 tile at the bottom centre of the sprite
	v2 TargetPos = NewPos;
	TargetPos.X += (fixed(Player->Width) >> 1) - 4; // the collision tile straddles two 'real' tiles on either side
	if (Player->Type == Entity_Player)
	{
		TargetPos.Y += Player->Height - 9;
	}
	else
	{
		TargetPos.Y += 2;
	}

	v2 TargetTilePos = PixelToTilePos(TargetPos);
	s32 TileLeft = TargetTilePos.X.WholePart;
	s32 TileRight = TileLeft + 1;
	s32 TileTop = TargetTilePos.Y.WholePart;
	s32 TileBottom = TileTop + 1;

	if (TileLeft < 0 || TileRight >= g_Level->TileWidth ||
		TileTop < 0 || TileBottom >= g_Level->TileHeight)
	{
		Result.Type = ColType_OutOfBounds;
		return Result;
	}
	else if (g_Camera.State == CamState_Fixed)
	{
		// Don't let player move outside camera view if fixed
		v2 ToCam = TargetPos - g_Camera.WorldPos;
		if (ToCam.X > SCREEN_WIDTH || ToCam.Y > SCREEN_HEIGHT ||
			ToCam.X < 0 || ToCam.Y < 0)
		{
			Result.Type = ColType_OutOfBounds;
			return Result;
		}
	}

	rect PlayerRect;
	PlayerRect.Pos = TargetPos;
	PlayerRect.Dimensions = iv2 { 7, 7 };
	
	// TODO: It's kind of lame how we're re-computing this here...
	u8 FacingDirs = DirectionsFromVector(NewPos - Player->WorldPos);
	const warp_tile* ClosestWarp = nullptr;
	fixed ClosestWarpDist = 1 << 22;
	for (u32 i = 0; i < g_Level->NumWarps; i++)
	{
		const warp_tile* Warp = g_Level->Warps + i;
		rect WarpRect;
		WarpRect.Pos = v2 { Warp->WorldPos.X, Warp->WorldPos.Y };
		WarpRect.Dimensions = iv2 { 8, 8 };
		
		b32 TouchedWarp = RectIntersect(&PlayerRect, &WarpRect);
		if (TouchedWarp)
		{
			fixed WarpDist = SqMagnitude(WarpRect.Pos - TargetPos);
			if (WarpDist < ClosestWarpDist &&
			    ((FacingDirs & Warp->Direction) || Warp->Direction == 0))
			{
				ClosestWarpDist = WarpDist;
				ClosestWarp = Warp;
			}
		}
	}
	if (ClosestWarp)
	{
		Result.Type = ColType_Warp;
		Result.TouchedWarp = ClosestWarp;
		return Result;
	}
	
	if (Player->Type == Entity_Player)
	{
		if (!(g_GameState->StateFlags & GameState_InBattle))
		{
			for (u32 i = 0; i < g_Level->NumScriptTiles; i++)
			{
				const script_tile* ScriptTile = g_Level->ScriptTiles + i;
				rect ScriptRect;
				ScriptRect.Pos = v2 { ScriptTile->WorldPos.X, ScriptTile->WorldPos.Y };
				ScriptRect.Dimensions = iv2 { 8, 8 };

				b32 TouchedScript = RectIntersect(&PlayerRect, &ScriptRect);
				if (TouchedScript && ScriptTile->TriggerValue == GetVar(ScriptTile->TriggerVar))
				{
					Result.Type = ColType_ScriptTile;
					Result.ScriptTile = ScriptTile;
					return Result;
				}
			}
		}
	}

	// Do entity collision first, so NPCs don't end up blocked by walls they are close to
	for (u32 i = 0; i < g_Level->NumEntities; i++)
	{
		entity* Entity = g_Level->Entities + i;
		if (Entity->IsInteractable())
		{
			rect OtherEntityRect = Entity->GetRect();

			if (RectIntersect(&PlayerRect, &OtherEntityRect))
			{
				Result.Type = ColType_Npc;
				Result.OtherEntity = Entity;
				return Result;
			}
		}
	}

	for (s32 TileY = TileTop; TileY <= TileBottom; TileY++)
	{
		for (s32 TileX = TileLeft; TileX <= TileRight; TileX++)
		{
			rect TileRect;
			TileRect.Pos = v2 { TileX << 3, TileY << 3 };
			TileRect.Dimensions = iv2(8, 8);

			u32 TileIndex = g_Level->TileWidth * TileY + TileX;
			Assert(TileIndex < g_Level->TileWidth * g_Level->TileHeight);

			const tile_collision TileCol = g_Level->CollisionMap[TileIndex];
			if (Player->Type == Entity_Player)
			{
				if (TileCol != TileCol_Walkable && RectIntersect(&PlayerRect, &TileRect))
				{
					if (TileCol == TileCol_WaterAdjacent && !g_Level->PlayerReflection->IsInteractable())
					{
						SpawnPlayerReflection(Player);
					}
					else if (TileCol == TileCol_Blocked || TileCol == TileCol_ReflectiveWater)
					{
						Result.Type = ColType_Wall;
						Result.TileX = TileX;
						Result.TileY = TileY;
					}
				}
			}
			else
			{
				Assert(Player->Type == Entity_Reflection);
				
				if (TileCol != TileCol_ReflectiveWater && RectIntersect(&PlayerRect, &TileRect))
				{
					Result.Type = ColType_Wall;
					Result.TileX = TileX;
					Result.TileY = TileY;
				}
			}
		}
	}

	return Result;
}

static void MovePlayer(entity* Player, entity* RealPlayer = nullptr)
{
	static constexpr fixed WalkSpeed = fixed(0.6);
	static constexpr fixed WalkDrag = fixed(0.4);
	static constexpr u32 DIR_CHANGE_DELAY = 5;
	static constexpr u32 LOITER_DELAY = 60;

	if (Player->StateFlags & EFlag_Locked)
	{
		if (!(g_GameState->StateFlags & GameState_InBattle))
		{
			SetAnimation(Player, nullptr);
		}
		Player->Velocity = {};
		return;
	}

	v2 InputDelta = {};
	if (g_Input->KeyDown(KEY_DOWN))
	{
		InputDelta.Y += 1;
	}
	if (g_Input->KeyDown(KEY_UP))
	{
		InputDelta.Y -= 1;
	}
	if (g_Input->KeyDown(KEY_LEFT))
	{
		InputDelta.X -= 1;
	}
	if (g_Input->KeyDown(KEY_RIGHT))
	{
		InputDelta.X += 1;
	}
	if (Player->Type == Entity_Reflection)
	{
		InputDelta.Y *= -1;
	}

	fixed DeltaMag = SqMagnitude(InputDelta);
	if (DeltaMag > 1)
	{
		Assert(DeltaMag == 2); // Vector can be max. 1 in x and y
		InputDelta *= INV_SQRT_2;
	}

	Player->DirChangeInertia = false;
	if (Player->Type == Entity_Player)
	{
		if (DeltaMag == 0)
		{
			if (g_Input->FramesSinceInput < LOITER_DELAY)
			{
				SetAnimation(Player, nullptr);
			}
			else
			{
				directional_anim DirAnim = s_DirectionalAnims[Player->FacingCardinal];
				SetAnimation(Player, DirAnim.LoiterAnim);
			}
		}
		else
		{
			Player->FacingVector = InputDelta;
			cardinal NewDirection = DeterminePlayerDirection(Player, InputDelta);
			if (Player->FacingCardinal != NewDirection)
			{
				Player->PrevFacingCardinal = Player->FacingCardinal;
			}
			Player->FacingCardinal = NewDirection;
			directional_anim DirAnim = s_DirectionalAnims[NewDirection];
			if (Player->PrevFacingCardinal != NewDirection &&
				g_Input->KeyHeldDownFrames(DirAnim.Button) < DIR_CHANGE_DELAY)
			{
				// First tap should only turn the player
				InputDelta = {};
				Player->DirChangeInertia = true;
			}
			else
			{
				Player->PrevFacingCardinal = NewDirection;
				SetAnimation(Player, DirAnim.WalkAnim);
			}
		}
	}
	else
	{
		Assert(Player->Type == Entity_Reflection && RealPlayer);
		SetAnimation(Player, RealPlayer->PlayingAnimation);
		Player->FacingVector = RealPlayer->FacingVector;
		Player->FacingVector.Y = -Player->FacingVector.Y;

		Player->FacingCardinal = RealPlayer->FacingCardinal;
		Player->PrevFacingCardinal = RealPlayer->PrevFacingCardinal;
		if (RealPlayer->DirChangeInertia)
		{
			// Piggy, but does the trick - make sure real player and reflection get same direction change inertia
			InputDelta = {};
		}
	}

	InputDelta *= WalkSpeed;
	InputDelta -= WalkDrag * Player->Velocity;

	v2 PlayerMoveDelta = fixed(0.5) * InputDelta + Player->Velocity;
	Player->Velocity += InputDelta;

	if (Abs(Player->Velocity.Y) < fixed(0.1))
	{
		Player->Velocity.Y = 0;
	}
	if (Abs(Player->Velocity.X) < fixed(0.1))
	{
		Player->Velocity.X = 0;
	}
	
	v2 NewPos = Player->WorldPos;
	NewPos.X += PlayerMoveDelta.X;

	collision_result Collision1 = WillCollide(Player, NewPos);
	if (Collision1.Type)
	{
		NewPos.X -= PlayerMoveDelta.X;
	}
	NewPos.Y += PlayerMoveDelta.Y;
	collision_result Collision2 = WillCollide(Player, NewPos);
	if (Collision2.Type)
	{
		NewPos.Y -= PlayerMoveDelta.Y;
	}

	Player->WorldPos = NewPos;
	ChangeOamEntryY(Player->Sprite->OamEntry, Player->WorldPos.Y.WholePart);

	if (Collision1.Type == ColType_Warp || Collision2.Type == ColType_Warp)
	{
		const warp_tile* Warp = Collision1.Type == ColType_Warp ? Collision1.TouchedWarp : Collision2.TouchedWarp;
		if (Warp->DestMap == Level_End)
		{
			if (GetFlag(ProgFlag_OrcDefeated))
			{
				g_GameState->OpenMenus = 0;
				g_GameState->StateFlags = GameState_EndSequence;
			}
			else
			{
				g_GameState->StateFlags &= ~GameState_PendingWarpTile;
			}
		}
		else
		{
			g_GameState->PendingWarpTile = Warp;
			g_GameState->StateFlags = GameState_PendingWarpTile; // NOTE: This is a rare case that completely resets game state...
		}
	}
	else if (Collision1.Type == ColType_ScriptTile || Collision2.Type == ColType_ScriptTile)
	{
		Assert(Player->Type == Entity_Player);

		const script_tile* ScriptTile = Collision1.Type == ColType_ScriptTile ? Collision1.ScriptTile : Collision2.ScriptTile;
		g_GameState->PendingScriptTile = ScriptTile;
		g_GameState->StateFlags |= GameState_PendingScriptTile;
	}

	//mgba_printf(LOG_INFO, "Player pos: (%d, %d)", Player->WorldPos.X.WholePart, Player->WorldPos.Y.WholePart);
}

static void PlayerWhiteOut(entity* Player)
{
	if (GetFlag(ProgFlag_TollGateCheckpoint))
	{
		g_GameState->PendingManualWarp.PendingWarpLevel = Level_TollGate;
		g_GameState->PendingManualWarp.PendingWarpLocation = iv2(62 * 8, 29 * 8);
	}
	else
	{
		g_GameState->PendingManualWarp.PendingWarpLevel = Level_JacksHouseGF;
		g_GameState->PendingManualWarp.PendingWarpLocation = iv2 { 17 * 8, 14 * 8 };
	}
	g_GameState->StateFlags |= GameState_PendingManualWarp;
	Player->Health = PLAYER_MAX_HEALTH;
	Player->FacingCardinal = Cardinal_Down;
}