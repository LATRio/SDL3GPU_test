#pragma once

class Entity;

class Component {
  protected:
    Entity *mOwner{};

  public:
    virtual ~Component() = default;

    virtual void Initialize() {}
    virtual void Update(float pDeltaTime) {}
    virtual void Render() {}
    virtual void RenderImGui() {}

    void SetOwner(Entity *pEntity) { mOwner = pEntity; }
    [[nodiscard]] Entity *GetOwner() const { return mOwner; }
};
