// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenlockedCustomTimeStep.h"
#include "GymEnvironmentTimeStep.generated.h"


/**
 * 
 */
UCLASS()
class CYBERTOOTHML_API UGymEnvironmentTimeStep : public UGenlockedCustomTimeStep
{
	GENERATED_BODY()
	
public:
	UGymEnvironmentTimeStep(const FObjectInitializer& ObjectInitializer);

	//~ UFixedFrameRateCustomTimeStep interface
	virtual bool Initialize(UEngine* InEngine) override;
	virtual void Shutdown(UEngine* InEngine) override;
	virtual bool UpdateTimeStep(UEngine* InEngine) override;
	virtual ECustomTimeStepSynchronizationState GetSynchronizationState() const override;
	virtual FFrameRate GetFixedFrameRate() const override;

	//~ UGenlockedCustomTimeStep interface
	virtual uint32 GetLastSyncCountDelta() const override;
	virtual bool IsLastSyncDataValid() const override;
	virtual bool WaitForSync() override;

public:

	/** Desired frame rate */
	UPROPERTY(EditAnywhere, Category = "Timing")
	FFrameRate FrameRate;

	/** Indicates that this custom time step should block to enforce the specified frame rate. Set to false if this is enforced elsewhere. */
	UPROPERTY(EditAnywhere, Category = "Timing")
	bool bShouldBlock;

	/** When true, delta time will always be 1/FrameRate, regardless of how much real time has elapsed */
	UPROPERTY(EditAnywhere, Category = "Timing")
	bool bForceSingleFrameDeltaTime;

	/** Should wait for gym environment step call from WS? */
	UPROPERTY(EditAnywhere, Category = "Timing")
	bool bWaitForStep;

	UPROPERTY()
	class URLManager* RLManager;

	


private:
	uint32 LastSyncCountDelta;
	double QuantizedCurrentTime;
	double LastIdleTime;


};
