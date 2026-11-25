#pragma once

class Scene;

class SceneManager {
    Scene *mCurrentScene{};

  public:
    static SceneManager *GetInstance();

    void Update(float pDelta) const;
    void Render() const;
    void RenderImGui() const;

    void ChangeScene(Scene *pNewScene);
    [[nodiscard]] Scene *GetCurrentScene() const { return mCurrentScene; }
};
