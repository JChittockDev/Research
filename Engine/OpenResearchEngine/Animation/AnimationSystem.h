#pragma once

class GameTimer;
class AssetManager;
struct FrameResource;

class AnimationSystem
{
public:
    void Update(const GameTimer& gt, AssetManager& assets, FrameResource& fr);
};
