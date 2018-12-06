#pragma once
#include "textures/textures.h"


struct FSoftwareTextureSpan
{
	uint16_t TopOffset;
	uint16_t Length;	// A length of 0 terminates this column
};


// For now this is just a minimal wrapper around FTexture. Once the software renderer no longer accesses FTexture directly, it is time for cleaning up.
class FSoftwareTexture
{
protected:
	FTexture *mTexture;
	FTexture *mSource;
	uint8_t *Pixels[2];
	std::vector<uint32_t> PixelsBgra;
	FSoftwareTextureSpan **Spandata[2] = { nullptr, nullptr };

	
public:
	FSoftwareTexture(FTexture *tex)
	{
		mTexture = tex;
		mSource = tex;
	}
	
	virtual ~FSoftwareTexture()
	{
		FreeAllSpans();
	}
	
	FTexture *GetTexture() const
	{
		return mTexture;
	}
	
	// The feature from hell... :(
	bool useWorldPanning() const
	{
		return mTexture->bWorldPanning;
	}

	bool isMasked()
	{
		return mTexture->bMasked;
	}
	
	bool UseBasePalette() const { return mTexture->UseBasePalette(); }
	int GetSkyOffset() const { return mTexture->GetSkyOffset(); }
	PalEntry GetSkyCapColor(bool bottom) const { return mTexture->GetSkyCapColor(bottom); }
	
	int GetWidth () { return mTexture->GetWidth(); }
	int GetHeight () { return mTexture->GetHeight(); }
	int GetWidthBits() { return mTexture->WidthBits; }
	int GetHeightBits() { return mTexture->HeightBits; }
	bool Mipmapped() { return mTexture->Mipmapped(); }

	int GetScaledWidth () { return mTexture->GetScaledWidth(); }
	int GetScaledHeight () { return mTexture->GetScaledHeight(); }
	double GetScaledWidthDouble () { return mTexture->GetScaledWidthDouble(); }
	double GetScaledHeightDouble () { return mTexture->GetScaledHeightDouble(); }
	double GetScaleY() const { return mTexture->GetScaleY(); }
	
	// Now with improved offset adjustment.
	int GetLeftOffset(int adjusted) { return mTexture->GetLeftOffset(adjusted); }
	int GetTopOffset(int adjusted) { return mTexture->GetTopOffset(adjusted); }
	int GetScaledLeftOffset (int adjusted) { return mTexture->GetScaledLeftOffset(adjusted); }
	int GetScaledTopOffset (int adjusted) { return mTexture->GetScaledTopOffset(adjusted); }
	double GetScaledLeftOffsetDouble(int adjusted) { return mTexture->GetScaledLeftOffsetDouble(adjusted); }
	double GetScaledTopOffsetDouble(int adjusted) { return mTexture->GetScaledTopOffsetDouble(adjusted); }
	
	// Interfaces for the different renderers. Everything that needs to check renderer-dependent offsets
	// should use these, so that if changes are needed, this is the only place to edit.
	
	// For the original software renderer
	int GetLeftOffsetSW() { return GetLeftOffset(r_spriteadjustSW); }
	int GetTopOffsetSW() { return GetTopOffset(r_spriteadjustSW); }
	int GetScaledLeftOffsetSW() { return GetScaledLeftOffset(r_spriteadjustSW); }
	int GetScaledTopOffsetSW() { return GetScaledTopOffset(r_spriteadjustSW); }
	
	// For the softpoly renderer, in case it wants adjustment
	int GetLeftOffsetPo() { return GetLeftOffset(r_spriteadjustSW); }
	int GetTopOffsetPo() { return GetTopOffset(r_spriteadjustSW); }
	int GetScaledLeftOffsetPo() { return GetScaledLeftOffset(r_spriteadjustSW); }
	int GetScaledTopOffsetPo() { return GetScaledTopOffset(r_spriteadjustSW); }
	
	DVector2 GetScale() const { return mTexture->Scale; }
	
	// Returns the whole texture, stored in column-major order
	virtual const uint8_t *GetPixels(FRenderStyle style)
	{
		return mTexture->GetPixels(style);
	}

	void Unload()
	{
		mTexture->Unload();
		PixelsBgra = std::vector<uint32_t>();
	}
	
	// Returns true if the next call to GetPixels() will return an image different from the
	// last call to GetPixels(). This should be considered valid only if a call to CheckModified()
	// is immediately followed by a call to GetPixels().
	virtual bool CheckModified (FRenderStyle style) { return false; }

	void GenerateBgraFromBitmap(const FBitmap &bitmap);
	void CreatePixelsBgraWithMipmaps();
	void GenerateBgraMipmaps();
	void GenerateBgraMipmapsFast();
	int MipmapLevels();
	void FreeAllSpans();
	
	FSoftwareTextureSpan **CreateSpans (const uint8_t *pixels);
	void FreeSpans (FSoftwareTextureSpan **spans);
	
	// Returns a single column of the texture
	virtual const uint8_t *GetColumn(FRenderStyle style, unsigned int column, const FSoftwareTextureSpan **spans_out);

	// Returns a single column of the texture, in BGRA8 format
	virtual const uint32_t *GetColumnBgra(unsigned int column, const FSoftwareTextureSpan **spans_out);

	// Returns the whole texture, stored in column-major order, in BGRA8 format
	virtual const uint32_t *GetPixelsBgra();

	
};

// A texture that returns a wiggly version of another texture.
class FWarpTexture : public FSoftwareTexture
{
	TArray<uint8_t> WarpedPixels[2];
	int bWarped = 0;
public:
	FWarpTexture (FTexture *source, int warptype);

	const uint32_t *GetPixelsBgra() override;
	bool CheckModified (FRenderStyle) override;

	float GetSpeed() const { return mTexture->shaderspeed; }

	uint64_t GenTime[2] = { 0, 0 };
	uint64_t GenTimeBgra = 0;
	int WidthOffsetMultiplier, HeightOffsetMultiplier;  // [mxd]
protected:

	const uint8_t *GetPixels(FRenderStyle style);
	int NextPo2 (int v); // [mxd]
	void SetupMultipliers (int width, int height); // [mxd]
};


inline FSoftwareTexture *FTexture::GetSoftwareTexture()
{
	if (!SoftwareTexture)
	{
		if (bWarped) SoftwareTexture = new FWarpTexture(this, bWarped);
		else SoftwareTexture = new FSoftwareTexture(this);
	}
	return SoftwareTexture;
}
