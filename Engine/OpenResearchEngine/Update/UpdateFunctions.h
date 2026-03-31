// Update/UpdateFunctions.h
#pragma once

class  AssetManager;
class  Camera;
struct FrameResource;
class  GameTimer;
struct SceneState;
class  ShadowResources;
class  Ssao;
class  Ssgi;
class  SSS;

void UpdateLights         (const GameTimer&, SceneState&, AssetManager&, FrameResource*);
void UpdateObjectCBs      (const GameTimer&, AssetManager&, FrameResource*);
void UpdateMaterialBuffer (const GameTimer&, AssetManager&, FrameResource*);
void UpdateShadowTransform(const GameTimer&, SceneState&);
void UpdateShadowPassCB   (const GameTimer&, SceneState&, const ShadowResources*, FrameResource*);
void UpdateMainPassCB     (const GameTimer&, const Camera&, SceneState&, FrameResource*);
void UpdateSssCB          (const GameTimer&, SceneState&, FrameResource*);
void UpdateScreenSpaceCB  (const GameTimer&, SceneState&, const Ssao*, const Ssgi*, const SSS*, FrameResource*);
void UpdateRadiancePassCB (const GameTimer&, SceneState&, AssetManager&, FrameResource*);
