#pragma once

namespace node
{
class SpaceScreenTransformer;
class IGraphicsSceneController;
class GraphicsObject;

class IGraphicsScene
{
public:
	virtual const SpaceScreenTransformer& GetSpaceScreenTransformer() const = 0;
	virtual void InvalidateRect() = 0;
	virtual bool IsObjectSelected(const GraphicsObject& object) const = 0;
};

}