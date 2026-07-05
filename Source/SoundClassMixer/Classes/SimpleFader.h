#pragma once

#include "DSP/VolumeFader.h"

/** Control-rate fader for managing volume fades of various standard shapes. */
class SOUNDCLASSMIXER_API FSimpleFader
{
public:
	FSimpleFader();

	/**
	 * Returns current volume of fader
	 */
	float GetVolume() const;

	/**
	 * Returns the volume given the delta from the current time
	 * into the future (Effectively like running to retrieve volume
	 * but without actually updating internal state).
	 */
	float GetVolumeAfterTime(float InDeltaTime) const;

	/**
	 * Returns the duration of the fade.
	 */
	float GetFadeDuration() const;

	/**
	 * Returns the curve type of the fader
	 */
	Audio::EFaderCurve GetCurve() const;

	/**
	 * Returns the target volume of the fader
	 */
	float GetTargetVolume() const;

	/**
	 * Returns whether or not the fader is currently
	 * fading over time.
	 */
	bool IsFading() const;

	/**
	 * Returns whether or not the fader is currently
	 * fading over time and value is increasing.
	 */
	bool IsFadingIn() const;

	/**
	 * Returns whether or not the fader is currently
	 * fading over time and value is decreasing.
	 */
	bool IsFadingOut() const;

	/**
	 * Sets the volume immediately, interrupting any currently active fade.
	 */
	void SetVolume(float InVolume);

	/**
	 * Applies a volume fade over time with the provided parameters.
	 */
	void StartFade(float InVolume, float InDuration, Audio::EFaderCurve InCurve);

	/**
	 * Stops fade, maintaining the current value as the target.
	 */
	void StopFade();

	/**
	 * Updates the fader's state with the given delta in time since last update.
	 */
	void Update(float InDeltaTime);

private:
	/** Converts value to final resulting volume */
	static float AlphaToVolume(float InAlpha, Audio::EFaderCurve InCurve);

	/** Current value used to linear interpolate over update delta
	  * (Normalized value for non-log, -80dB to 0dB for log)
	  */
	float CurrentVolume;

	/** Target value used to linear interpolate over update delta
	  * (Normalized value for non-log, -80dB to 0dB for log)
	  */
	float TargetVolume;

	/** Duration fader is to perform fade */
	float FadeDuration;

	/** Time elapsed since fade has been initiated */
	float ElapsedTime;

	/** Audio fader curve to use */
	Audio::EFaderCurve FadeCurve;
};