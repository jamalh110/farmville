//
//  CUPolygonObstacle.cpp
//  Cornell Extensions to Cocos2D
//
//  This class implements a polygonal Physics object.  This is different from
//  PolygonNode, which is used for drawing.  This class is substantially more
//  complex than the other physics objects, but it will allow you to draw
//  arbitrary shapes.  Be careful modifying this file as there are a lot of
//  subtleties here.
//
//  This class uses our standard shared-pointer architecture.
//
//  1. The constructor does not perform any initialization; it just sets all
//     attributes to their defaults.
//
//  2. All initialization takes place via init methods, which can fail if an
//     object is initialized more than once.
//
//  3. All allocation takes place via static constructors which return a shared
//     pointer.
//
//  CUGL MIT License:
//      This software is provided 'as-is', without any express or implied
//      warranty.  In no event will the authors be held liable for any damages
//      arising from the use of this software.
//
//      Permission is granted to anyone to use this software for any purpose,
//      including commercial applications, and to alter it and redistribute it
//      freely, subject to the following restrictions:
//
//      1. The origin of this software must not be misrepresented; you must not
//      claim that you wrote the original software. If you use this software
//      in a product, an acknowledgment in the product documentation would be
//      appreciated but is not required.
//
//      2. Altered source versions must be plainly marked as such, and must not
//      be misrepresented as being the original software.
//
//      3. This notice may not be removed or altered from any source distribution.
//
//  This file is based on the CS 3152 PhysicsDemo Lab by Don Holden, 2007
//
//  Author: Walker White
//  Version: 7/3/24 (CUGL 3.0 reorganization)
//
#include <box2d/b2_polygon_shape.h>
#include <cugl/physics2/CUPolygonObstacle.h>

using namespace cugl::physics2;

/** The epsilon necessary for box2d not to mark as degenerate */
#define EPSILON 0.01

/**
 * Returns true if the given vertices are a non-degenerate triangle
 *
 * This method will return false if the vertices are colinear.
 *
 * @param verts The array of vertices
 * @param count The number of vertices
 *
 * @return true if the given vertices are a non-degenerate triangle
 */
static bool valid_shape(const b2Vec2* verts, int count) {
    CUAssertLog(count == 3, "This validator currently only supports triangles");
    const b2Vec2* p = verts;
    const b2Vec2* q = verts+1;
    const b2Vec2* r = verts+2;
    if (fabsf(p->x * (q->y - r->y) + q->x * (r->y - p->y) + r->x * (p->y - q->y)) <= EPSILON) {
        return false;
    }
    
    // Check that box2d can perform welding
    b2Vec2 ps[3];
    int32 tempCount = 0;
    for (int32 i = 0; i < count; ++i) {
        b2Vec2 v = verts[i];

        bool unique = true;
        for (int32 j = 0; j < tempCount; ++j) {
            if (b2DistanceSquared(v, ps[j]) < ((0.5f * b2_linearSlop) * (0.5f * b2_linearSlop))) {
                unique = false;
                break;
            }
        }

        if (unique) {
            ps[tempCount++] = v;
        }
    }

    return tempCount == 3;
}

#pragma mark -
#pragma mark Constructors
/**
 * Initializes a (not necessarily convex) polygon
 *
 * The given polygon defines an implicit coordinate space. The body (and hence
 * the rotational center) will be placed at the given origin position.
 *
 * @param poly   The polygon vertices
 * @param origin The rotational center with respect to the vertices
 *
 * @return  true if the obstacle is initialized properly, false otherwise.
 */
bool PolygonObstacle::init(const Poly2& poly, const Vec2 origin) {
    Obstacle::init(Vec2::ZERO);
        
    // Compute anchor from absolute origin
    _bodyinfo.position.Set(origin.x,origin.y);
    Rect bounds = poly.getBounds();
    _anchor.x = (origin.x-bounds.origin.x)/bounds.size.width;
    _anchor.y = (origin.y-bounds.origin.y)/bounds.size.height;
    setPolygon(poly);
    return true;
}

/**
 * Initializes a (not necessarily convex) polygon
 *
 * The anchor point (the rotational center) of the polygon is specified as a
 * ratio of the bounding box.  An anchor point of (0,0) is the bottom left of
 * the bounding box.  An anchor point of (1,1) is the top right of the bounding
 * box.  The anchor point does not need to be contained with the bounding box.
 *
 * @param  poly     The polygon vertices
 * @param  anchor   The rotational center of the polygon
 *
 * @return  true if the obstacle is initialized properly, false otherwise.
 */
bool PolygonObstacle::initWithAnchor(const Poly2& poly, const Vec2 anchor) {
    Obstacle::init(Vec2::ZERO);
    
    // Compute the position from the anchor point
    Vec2 pos = poly.getBounds().origin;
    pos.x += anchor.x*poly.getBounds().size.width;
    pos.y += anchor.x*poly.getBounds().size.height;
    
    _bodyinfo.position.Set(pos.x,pos.y);
    _anchor = anchor;
    setPolygon(poly);
    return true;
}

/**
 * Deletes this physics object and all of its resources.
 *
 * A non-default destructor is necessary since we must release all
 * the fixture pointers for the polygons.
 */
PolygonObstacle::~PolygonObstacle() {
    CUAssertLog(_body == nullptr, "You must deactive physics before deleting an object");
    if (_shapes != nullptr) {
        delete[] _shapes;
        _shapes = nullptr;
    }
    if (_geoms != nullptr) {
        delete[] _geoms;
        _geoms = nullptr;
    }
}


#pragma mark -
#pragma mark Resizing
/**
 * Resets the polygon vertices in the shape to match the dimension.
 *
 * This is an internal method and it does not mark the physics object as dirty.
 *
 * @param  size The new dimension (width and height)
 */
void PolygonObstacle::resize(const Size size) {
    // Need to do two things:
    // 1. Adjust the polygon.
    // 2. Update the shape information
    float origwide = _polygon.getBounds().size.width;
    float orighigh = _polygon.getBounds().size.height;
    _polygon *= Vec2(size.width/origwide, size.height/orighigh);
    if (_debug != nullptr) {
        resetDebug();
    }
}

/**
 * Recreates the shape objects attached to this polygon.
 *
 * This must be called whenever the polygon is resized.
 */
void PolygonObstacle::resetShapes() {
    int ntris =  (int)_polygon.indices.size() / 3;
    if (_shapes != nullptr) {
        delete[] _shapes;
    }
    
    Vec2 pos = getPosition();
    _shapes = new b2PolygonShape[ntris];
    b2Vec2 triangle[3];
    int index = 0;
    for(int ii = 0; ii < ntris; ii++) {
        for(int jj = 0; jj < 3; jj++) {
            Uint32 ind = _polygon.indices[3*ii+jj];
            Vec2 temp = _polygon.vertices[ind]-pos;
            triangle[jj].x = temp.x;
            triangle[jj].y = temp.y;
        }
        // Only add non-degenerate triangles
        if (valid_shape(triangle, 3)) {
        	// Box2d latest is more agressive than valid_shape
            if (_shapes[index].Set(triangle,3)) {
            	index++;
            }
        }
    }
    ntris = index;
    
    if (_geoms == nullptr) {
        _geoms = new b2Fixture*[ntris];
        for(int ii = 0; ii < ntris; ii++) { _geoms[ii] = nullptr; }
        _fixCount = ntris;
    } else {
        markDirty(true);
    }
}


#pragma mark -
#pragma mark Dimensions

/**
 * Sets the rotational center of this polygon
 *
 * The anchor point of the polygon is specified as ratio of the bounding
 * box.  An anchor point of (0,0) is the bottom left of the bounding box.
 * An anchor point of (1,1) is the top right of the bounding box.  The
 * anchorpoint does not need to be contained with the bounding box.
 *
 * @param x  the x-coordinate of the rotational center
 * @param y  the y-coordinate of the rotational center
 */
void PolygonObstacle::setAnchor(float x, float y) {
    _anchor.set(x,y);
    
    // Compute the position from the anchor point
    Vec2 pos = _polygon.getBounds().origin;
    pos.x += x*_polygon.getBounds().size.width;
    pos.y += x*_polygon.getBounds().size.height;
    setPosition(pos.x,pos.y);
    resetShapes();
}

/**
 * Sets the polygon defining this object
 *
 * This change cannot happen immediately.  It must wait until the
 * next update is called.
 *
 * @param poly   the polygon defining this object
 */
void PolygonObstacle::setPolygon(const Poly2& poly) {
    _polygon.set(poly);
    resetShapes();
}


#pragma mark -
#pragma mark Scene Graph Methods
/**
 * Creates the outline of the physics fixtures in the debug node
 *
 * The debug node is use to outline the fixtures attached to this object.
 * This is very useful when the fixtures have a very different shape than
 * the texture (e.g. a circular shape attached to a square texture).
 */
void PolygonObstacle::resetDebug() {
    if (_debug == nullptr) {
        _debug = scene2::WireNode::allocWithTraversal(_polygon,poly2::Traversal::INTERIOR);
        _debug->setColor(_dcolor);
        if (_scene != nullptr) {
            _scene->addChild(_debug);
        }
    } else {
        _debug->setTraversal(poly2::Traversal::INTERIOR);
        _debug->setPolygon(_polygon);
    }
    _debug->setAnchor(_anchor);
    _debug->setPosition(getPosition());
}


#pragma mark -
#pragma mark Physics Methods

/**
 * Create new fixtures for this body, defining the shape
 *
 * This is the primary method to override for custom physics objects
 */
void PolygonObstacle::createFixtures() {
    if (_body == nullptr) {
        return;
    }
    
    // Create the fixtures
    releaseFixtures();
    if (_geoms == nullptr) {
        _geoms = new b2Fixture*[_fixCount];
    }
    for(int ii = 0; ii < _fixCount; ii++) {
        _fixture.shape = &(_shapes[ii]);
        _geoms[ii] = _body->CreateFixture(&_fixture);
    }
    markDirty(false);
}

/**
 * Release the fixtures for this body, reseting the shape
 *
 * This is the primary method to override for custom physics objects
 */
void PolygonObstacle::releaseFixtures() {
    if (_geoms != nullptr && _geoms[0] != nullptr) {
        for(int ii = 0; ii < _fixCount; ii++) {
            _body->DestroyFixture(_geoms[ii]);
            _geoms[ii] = nullptr;
        }
    }
    if (_geoms != nullptr) {
        delete[] _geoms;
        _geoms = nullptr;
    }
}
