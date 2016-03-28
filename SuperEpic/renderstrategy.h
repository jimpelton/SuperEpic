#ifndef epic_renderstrategy_h__
#define epic_renderstrategy_h__

#include "renderer.h"

struct SDL_Renderer;

class RenderStrategy
{
public:
  RenderStrategy();
  virtual ~RenderStrategy();

  virtual void render(SDL_Renderer *renderer) = 0;

private:
  const Renderer *m_renderer;
};

class GalleryRenderStrategy : RenderStrategy
{
  GalleryRenderStrategy();
  virtual ~GalleryRenderStrategy();

  void render(SDL_Renderer *renderer) override;


private:
  void renderTextures();



};

#endif // !epic_renderstrategy_h__
