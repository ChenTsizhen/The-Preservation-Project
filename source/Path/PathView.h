//
//  PathView.h
//  Tilemap
//
//  Created by Yichen Qiu on 2/23/23.
//

#ifndef PathView_h
#define PathView_h
#include <cugl/cugl.h>


using namespace cugl;

class PathView{
private:
    
    std::vector<std::shared_ptr<scene2::PolygonNode>> _pathLines;
    
public:
    PathView(std::vector<std::shared_ptr<scene2::PolygonNode>> pathLines){
        _pathLines = pathLines;
    }
    
    
    void addToPathLines(Spline2 spline, Vec2 pos, const std::shared_ptr<cugl::Scene2>& scene){
        SplinePather splinePather = SplinePather();
        SimpleExtruder extruder = SimpleExtruder();
        
        splinePather.set(&spline);
        splinePather.calculate();
        
        extruder.set(splinePather.getPath());
        extruder.calculate(5);
        Poly2 line = extruder.getPolygon();
        
        std::shared_ptr<scene2::PolygonNode> polyNode= scene2::PolygonNode::alloc();
        polyNode->setPolygon(line);
        polyNode->setPosition(pos);
        polyNode->setColor(Color4::BLACK);
        _pathLines.push_back(polyNode);
        scene->addChild(polyNode);
        
    }
    
    
    void clearPathLines(){
        _pathLines.clear();
    }
    
    void removeChildren(const std::shared_ptr<cugl::Scene2>& scene){
        for (int i = 0; i < _pathLines.size(); i++){
            scene->removeChild(_pathLines[i]);
        }
        _pathLines.clear();
    }
    
};

#endif /* PathView_h */
