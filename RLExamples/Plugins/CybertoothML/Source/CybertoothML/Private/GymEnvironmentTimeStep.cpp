// Fill out your copyright notice in the Description page of Project Settings.


#include "CybertoothML/GymEnvironmentTimeStep.h"
#include "Kismet/GameplayStatics.h"
#include "CybertoothML/Server/ServerWebSocketSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "CybertoothML/RLManager.h"




UGymEnvironmentTimeStep::UGymEnvironmentTimeStep(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, FrameRate(24, 1)
	, bShouldBlock(false)
	, bForceSingleFrameDeltaTime(true)
	, LastSyncCountDelta(0)
	, QuantizedCurrentTime(0.0)
{
	bWaitForStep = false;
}

bool UGymEnvironmentTimeStep::Initialize(UEngine* InEngine)
{
	UE_LOG(LogTemp, Warning, TEXT("*********** INIT STEPS v3"));

	bShouldBlock = FParse::Param(FCommandLine::Get(), TEXT("human"));

	return true;
}

void UGymEnvironmentTimeStep::Shutdown(UEngine* InEngine)
{
	// Empty but implemented because it is PURE_VIRTUAL
}

bool UGymEnvironmentTimeStep::UpdateTimeStep(UEngine* InEngine)
{
	// Find websocket server Subsystem
	if (IsValid(RLManager) == false && GEngine)
	{
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			UWorld* World = Context.World();
			if (World && World->IsGameWorld() && World->GetGameInstance())
			{
				RLManager = World->GetGameInstance()->GetSubsystem<URLManager>();
				break;
			}
		}
	}

	/*
	if (IsValid(RLManager) && RLManager->StepState == ERLStepState::PerformingTick)
	{
		RLManager->FinishStep();
	}
	*/

	UpdateApplicationLastTime(); // Copies "CurrentTime" (used during the previous frame) in "LastTime"
	WaitForSync();
	UpdateAppTimes(QuantizedCurrentTime - LastIdleTime, QuantizedCurrentTime);

	return false; // false means that the Engine's TimeStep should NOT be performed.
}

ECustomTimeStepSynchronizationState UGymEnvironmentTimeStep::GetSynchronizationState() const
{
	return ECustomTimeStepSynchronizationState::Synchronized;
}

FFrameRate UGymEnvironmentTimeStep::GetFixedFrameRate() const
{
	return FrameRate;
}

uint32 UGymEnvironmentTimeStep::GetLastSyncCountDelta() const
{
	return LastSyncCountDelta;
}

bool UGymEnvironmentTimeStep::IsLastSyncDataValid() const
{
	return true;
}

bool UGymEnvironmentTimeStep::WaitForSync()
{
	const double FramePeriod = GetFixedFrameRate().AsInterval();
	double CurrentPlatformTime = FPlatformTime::Seconds();
	double DeltaRealTime = CurrentPlatformTime - FApp::GetLastTime();

	// Handle the unexpected case of a negative DeltaRealTime by forcing LastTime to CurrentPlatformTime.
	if (DeltaRealTime < 0)
	{
		FApp::SetCurrentTime(CurrentPlatformTime); // Necessary since we don't have direct access to FApp's LastTime
		FApp::UpdateLastTime();
		DeltaRealTime = CurrentPlatformTime - FApp::GetLastTime(); // DeltaRealTime should be zero now, which will force a sleep
	}

	checkSlow(DeltaRealTime >= 0);

	LastIdleTime = FramePeriod - DeltaRealTime;	

	if(false)//if (bShouldBlock || (RLManager && WebsocketServer->GetHumanMode()))
	{
		// Sleep during the idle time
		if (LastIdleTime > 0.f)
		{
			// Normal sleep for the bulk of the idle time.
			if (LastIdleTime > 5.f / 1000.f)
			{
				FPlatformProcess::SleepNoStats(LastIdleTime - 0.002f);
			}

			// Give up timeslice for small remainder of wait time.

			const double WaitEndTime = FApp::GetLastTime() + FramePeriod;

			while (FPlatformTime::Seconds() < WaitEndTime)
			{
				FPlatformProcess::SleepNoStats(0.f);
			}

			// Current platform time should now be right after the desired WaitEndTime, with an overshoot
			CurrentPlatformTime = FPlatformTime::Seconds();
			FApp::SetIdleTimeOvershoot(CurrentPlatformTime - WaitEndTime);

			// Update DeltaRealTime now that we've slept enough
			DeltaRealTime = CurrentPlatformTime - FApp::GetLastTime();
		}
	}
	else {
		// AI driven

		/*
		if (IsValid(RLManager))
		{
			RLManager->WaitForStep();
		}
		*/

		/*
		if (IsValid(WebsocketServer))
		{
			// Wait until we have a message
			WebsocketServer->Tick();
			while (WebsocketServer->Messages.IsEmpty())
			{
				FPlatformProcess::SleepNoStats(0.002f);
				WebsocketServer->Tick();
			}

			// Process all new messages
			while (WebsocketServer->Messages.IsEmpty() == false)
			{
				FString Message;
				bool bSuccess = WebsocketServer->Messages.Dequeue(Message);
				if (!bSuccess)
				{
					break;
				}

				WebsocketServer->ProcessMessage(Message);
			}
		}
		*/
	}


	// This +1e-4 avoids a case of LastSyncCountData incorrectly ending up as 0.
	DeltaRealTime += 1e-4;

	// Quantize elapsed frames, capped to the maximum that the integer type can hold.
	LastSyncCountDelta = uint32(FMath::Min(FMath::Floor(DeltaRealTime / FramePeriod), double(MAX_uint32)));

	if (bShouldBlock)
	{
		ensure(LastSyncCountDelta > 0);
	}
	else if (LastSyncCountDelta < 1)
	{
		LastSyncCountDelta = 1;
	}

	if (bForceSingleFrameDeltaTime)
	{
		LastSyncCountDelta = 1;
	}

	// Save quantized current time for use outside this function.
	QuantizedCurrentTime = FApp::GetLastTime() + LastSyncCountDelta * FramePeriod;

	return true;
}

