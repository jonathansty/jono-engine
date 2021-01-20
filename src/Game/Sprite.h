#pragma once
//-----------------------------------------------------
// Name: Steyfkens
// First name: Jonathan
// Group: 1DAE01
//-----------------------------------------------------

class Sprite 
{
public:
	Sprite(String spritesheetPath, int amountRows, int amountCols, int maxFrame);
	virtual ~Sprite( );

	Sprite( const Sprite& ) = delete;
	Sprite& operator=( const Sprite& ) = delete;

    void Paint(graphics::D2DRenderContext& ctx, int rowNumber);
    void Paint(graphics::D2DRenderContext& ctx, int rowNumber, int maxFrame);
    void Tick(double deltaTime);
    void SetFrameRate(double frameRate);

private: 

    int m_Rows = 0;
    int m_Cols = 0;
    int m_MaxFrame = 0;
    int m_ClipWidth = 0;
    int m_ClipHeight = 0;
    double m_FrameRate = 12.0;
    double m_AccuTime = 0;
    int m_FrameNr = 0;
    Bitmap* m_BmpSpriteSheetPtr = nullptr;
};

 
