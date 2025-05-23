//
//  CUSceneLoader.cpp
//  Cornell University Game Library (CUGL)
//
//  This module provides a specific implementation of the Loader class to load
//  a 2d scene graph. Scene graphs are always specified as a JSON tree. This
//  is constantly evolving, as we continue to add new node types and layout
//  managers.
//
//  As with all of our loaders, this loader is designed to be attached to an
//  asset manager.  In addition, this class uses our standard shared-pointer
//  architecture.
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
//  Author: Walker White
//  Version: 7/3/24 (CUGL 3.0 reorganization)
//
#include <cugl/core/assets/CUAssetManager.h>
#include <cugl/core/assets/CUWidgetValue.h>
#include <cugl/scene2/cu_scene2.h>
#include <cugl/core/io/CUJsonReader.h>
#include <cugl/core/util/CUStringTools.h>
#include <cugl/core/util/CUFiletools.h>
#include <cugl/core/CUApplication.h>
#include <cugl/core/CUDisplay.h>
#include <locale>
#include <algorithm>

using namespace cugl;
using namespace cugl::scene2;
using namespace cugl::graphics;

/** If the type is unknown */
#define UNKNOWN_STR  "<unknown>"

/**
 * Initializes a new asset loader.
 *
 * This method bootstraps the loader with any initial resources that it
 * needs to load assets. Attempts to load an asset before this method is
 * called will fail.
 *
 * This method is abstract and should be overridden in the specific
 * implementation for each asset.
 *
 * @param threads   The thread pool for asynchronous loading support
 *
 * @return true if the asset loader was initialized successfully
 */
bool Scene2Loader::init(const std::shared_ptr<ThreadPool>& threads) {
    _loader=threads;
    
    // Define the supported types
    _types["node"] = Widget::NODE;
    _types["image"] = Widget::IMAGE;
    _types["solid"] = Widget::SOLID;
    _types["polygon"] = Widget::POLY;
    _types["path"] = Widget::PATH;
    _types["wireframe"] = Widget::WIRE;
    _types["wire frame"] = Widget::WIRE;
    _types["sprite"] = Widget::ANIMATE;
    _types["order"] = Widget::ORDER;
    _types["canvas"] = Widget::CANVAS;
    _types["ninepatch"] = Widget::NINE;
    _types["label"] = Widget::LABEL;
    _types["button"] = Widget::BUTTON;
    _types["buttongroup"] = Widget::BUTTONGROUP;
    _types["progress"] = Widget::PROGRESS;
    _types["slider"] = Widget::SLIDER;
    _types["scroll"] = Widget::SCROLL;
    _types["scroll pane"] = Widget::SCROLL;
    _types["textfield"] = Widget::TEXTFIELD;
    _types["text field"] = Widget::TEXTFIELD;
	_types["widget"] = Widget::EXTERNAL_IMPORT;

    // Define the supported layouts
    _forms["none"] = Form::NONE;
    _forms["absolute"] = Form::NONE;
    _forms["anchored"] = Form::ANCHORED;
    _forms["float"]  = Form::FLOAT;
    _forms["grid"] = Form::GRID;

    return true;
}

/**
 * Recursively builds the scene from the given JSON tree.
 *
 * This method allows us to maximize the asynchronous creation of scenes.
 * The key is assigned as the name of the root Node of the scene.
 *
 * The JSON tree should be a tree of widget objects, where each widget
 * object has the following attribute values:
 *
 *      "type":     The node type (a Node or any subclass)
 *      "data":     Data (images, labels) that define the widget.  This
 *                  JSON object has a node-specific format.
 *      "format":   The layout manager to use for this Node. This layout
 *                  manager will apply to all the children (see below).
 *                  This JSON object has a layout-specific format.
 *      "layout":   Node placement using a the layout manager of the parent.
 *                  This is applied after parsing "data' and will override
 *                  any settings there. This JSON object has a
 *                  layout-specific format.
 *      "children": Any child Nodes of this one. This JSON object has a
 *                  named attribute for each child.
 *
 * With the exception of "type", all of these attributes are JSON objects.
 *
 * @param key       The key to access the scene after loading
 * @param json      The JSON object defining the scene
 *
 * @return the SDL_Surface with the texture information
 */
std::shared_ptr<scene2::SceneNode> Scene2Loader::build(const std::string key,
                                                      const std::shared_ptr<JsonValue>& json) const {
    std::string type = json->getString("type",UNKNOWN_STR);
    auto it = _types.find(cugl::strtool::tolower(type));
    if (it == _types.end()) {
        return nullptr;
    }
    
    const AssetManager* manager = getManager();
    
    bool nonrelative = false;
    std::shared_ptr<JsonValue> data = json->get("data");
    std::shared_ptr<scene2::SceneNode> node = nullptr;
    switch (it->second) {
    case Widget::NODE:
        node = scene2::SceneNode::allocWithData(manager,data);
        break;
    case Widget::IMAGE:
        node = scene2::PolygonNode::allocWithData(manager,data);
        break;
    case Widget::SOLID:
        // TODO: Replace with polygon as first child and sized to fill
        // That will keep us from breaking the tint abstraction
        node = scene2::PolygonNode::allocWithData(manager,data);
        nonrelative = true;
        break;
    case Widget::POLY:
        node = scene2::PolygonNode::allocWithData(manager,data);
        break;
    case Widget::PATH:
        node = scene2::PathNode::allocWithData(manager,data);
        break;
    case Widget::WIRE:
        node = scene2::WireNode::allocWithData(manager,data);
        break;
    case Widget::ORDER:
        node = scene2::OrderedNode::allocWithData(manager,data);
        break;
    case Widget::CANVAS:
        node = scene2::CanvasNode::allocWithData(manager,data);
        break;
    case Widget::ANIMATE:
        node = scene2::SpriteNode::allocWithData(manager,data);
        break;
    case Widget::NINE:
        node = scene2::NinePatch::allocWithData(manager,data);
        break;
    case Widget::LABEL:
        node = scene2::Label::allocWithData(manager,data);
        break;
    case Widget::BUTTON:
        node = scene2::Button::allocWithData(manager,data);
        break;
    case Widget::BUTTONGROUP:
        node = scene2::ButtonGroup::allocWithData(manager, data);
        break;
    case Widget::PROGRESS:
        node = scene2::ProgressBar::allocWithData(manager,data);
        break;
    case Widget::SLIDER:
        node = scene2::Slider::allocWithData(manager,data);
        break;
    case Widget::SCROLL:
        node = scene2::ScrollPane::allocWithData(manager,data);
        break;
    case Widget::TEXTFIELD:
        node = scene2::TextField::allocWithData(manager,data);
        break;
	case Widget::EXTERNAL_IMPORT:
    {
		const std::shared_ptr<JsonValue> widgetJson = getWidgetJson(json);
		return build(key, widgetJson);
	}
    case Widget::UNKNOWN:
        break;
    }
    
    if (node == nullptr) {
        return nullptr;
    }
    
    if (node->getContentSize() == Size::ZERO) {
        node->setAnchor(Vec2::ANCHOR_BOTTOM_LEFT);
        node->setContentSize(Display::get()->getBounds().size);
    }
    
    
    std::shared_ptr<JsonValue> form = json->get("format");
    std::string ftype =  (form == nullptr ? UNKNOWN_STR : form->getString("type",UNKNOWN_STR));
    auto jt = _forms.find(cugl::strtool::tolower(ftype));
    
    std::shared_ptr<scene2::Layout> layout = nullptr;
    if (jt != _forms.end()) {
        switch (jt->second) {
        case Form::ANCHORED:
            layout = scene2::AnchoredLayout::allocWithData(form);
            break;
        case Form::FLOAT:
            layout = scene2::FloatLayout::allocWithData(form);
            break;
        case Form::GRID:
            layout = scene2::GridLayout::allocWithData(form);
            break;
        case Form::NONE:
        case Form::UNKNOWN:
            break;
        }
    }
    node->setLayout(layout);
    
    std::shared_ptr<JsonValue> children = json->get("children");
	if (children != nullptr) {
		for (int ii = 0; ii < children->size(); ii++) {
			std::shared_ptr<JsonValue> item = children->get(ii);
			std::string key = item->key();
			if (key != "comment") {
				// If this is a widget, use the loaded widget json instead
				if (item->has("type") && item->getString("type") == "Widget") {
					item = getWidgetJson(item);
				}

				std::shared_ptr<scene2::SceneNode> kid = build(key, item);
                if (nonrelative) {
                    kid->setRelativeColor(false);
                }
				node->addChild(kid);

				if (layout != nullptr && item->has("layout")) {
					std::shared_ptr<JsonValue> posit = item->get("layout");
					layout->add(key, posit);
				}
			}
		}
	}
    
    // Do not perform layout yet.
    node->setName(key);
    return node;
}


/**
 * Translates the JSON of a widget to the JSON of the node that it encodes.
 *
 * If this scene is built before the JSON of any used widgets have been loaded, this will fail.
 *
 * @param json      The JSON object specifying the widget's file and the values for its exposed variables
 *
 * @return the JSON loaded from the widget file with all variables set based on the values presented in json.
 */
std::shared_ptr<JsonValue> Scene2Loader::getWidgetJson(const std::shared_ptr<JsonValue>& json) const {
	std::shared_ptr<JsonValue> data = json->get("data");
	std::string widgetSource = data->getString("key");
	std::shared_ptr<JsonValue> widgetVars = data->get("variables");
	std::shared_ptr<JsonValue> layout = json->get("layout");

	const std::shared_ptr<WidgetValue> widget = _manager->get<WidgetValue>(widgetSource);
	CUAssertLog(widget != nullptr, "No widget found with name %s", widgetSource.c_str());
    std::shared_ptr<JsonValue> contentCopy = widget->substitute(widgetVars);

    /**
	const std::shared_ptr<JsonValue> widgetJson = widget->getJson();

	std::shared_ptr<JsonValue> variables = widgetJson->get("variables");
	std::shared_ptr<JsonValue> contents = widgetJson->get("contents");
	std::string contentString = contents->toString();
	std::shared_ptr<JsonValue> contentCopy = JsonValue::allocWithJson(contentString);
    if (widgetVars) {
        for (int ii = 0; ii < widgetVars->size(); ii++) {
            auto child = widgetVars->get(ii);
            if (variables->has(child->key())) {
                bool found = true;
                std::shared_ptr<JsonValue> address = variables->get(child->key());
                std::shared_ptr<JsonValue> spotToChange = contentCopy;
                std::vector<std::string> sAry = address->asStringArray();
                for (std::string s : sAry) {
                    if (spotToChange->has(s)) {
                        spotToChange = spotToChange->get(s);
                    }
                    else {
                        found = false;
                    }
                }
                if (found) {
                    spotToChange->merge(child);
                } else {
                    std::string err = "No variable found within widget " + widgetSource + " matching name " + child->key();
                    CULogError("%s",err.c_str());
                }
            }
        }
	}
     */

	// reassign the layout if it exists
	if (layout != nullptr) {
		std::shared_ptr<JsonValue> contentsLayout = contentCopy->get("layout");
		if (contentsLayout == nullptr) {
			contentCopy->appendChild("layout", std::make_shared<JsonValue>());
			contentsLayout = contentCopy->get("layout");
		}
		contentsLayout->merge(layout);
	}

	// now recursively check to see if this was a widget
	if (contentCopy->has("type") && contentCopy->getString("type") == "Widget") {
		return getWidgetJson(contentCopy);
	}
	return contentCopy;
}


/**
 * Records the given Node with this loader, so that it may be unloaded later.
 *
 * This method finishes the asset loading started in {@link preload}. This
 * step is not safe to be done in a separate thread, as it accesses the
 * main asset table.  Therefore, it takes place in the main CUGL thread
 * via {@link Application#schedule}.  The scene is stored using the name
 * of the root Node as a key.
 *
 * This method supports an optional callback function which reports whether
 * the asset was successfully materialized.
 *
 * @param node      The scene asset
 * @param callback  An optional callback for asynchronous loading
 *
 * @return true if materialization was successful
 */
bool Scene2Loader::materialize(const std::shared_ptr<scene2::SceneNode>& node, LoaderCallback callback) {
    bool success = false;
    
    std::string key = "";
    if (node != nullptr) {
        key = node->getName();
        success = attach(key, node);
    }

    if (callback != nullptr) {
        callback(key,success);
    }
    _queue.erase(key);
    return success;
}


/**
 * Internal method to support asset loading.
 *
 * This method supports either synchronous or asynchronous loading, as
 * specified by the given parameter.  If the loading is asynchronous,
 * the user may specify an optional callback function.
 *
 * This method will split the loading across the {@link preload} and
 * {@link materialize} methods.  This ensures that asynchronous loading
 * is safe.
 *
 * @param key       The key to access the asset after loading
 * @param source    The pathname to the asset
 * @param size      The font size (overriding the default)
 * @param callback  An optional callback for asynchronous loading
 * @param async     Whether the asset was loaded asynchronously
 *
 * @return true if the asset was successfully loaded
 */
bool Scene2Loader::read(const std::string key, const std::string source,
                        LoaderCallback callback, bool async) {
    if (_assets.find(key) != _assets.end() || _queue.find(key) != _queue.end()) {
        return false;
    }

    // Make sure we reference the asset directory
    bool absolute = cugl::filetool::is_absolute(source);
    CUAssertLog(!absolute, "This loader does not accept absolute paths for assets");

    std::string root = Application::get()->getAssetDirectory();
    std::string path = root+source;
    
    bool success = false;
    if (_loader == nullptr || !async) {
        enqueue(key);
        std::shared_ptr<JsonReader> reader = JsonReader::allocWithAsset(path);
        std::shared_ptr<JsonValue> json = (reader == nullptr ? nullptr : reader->readJson());
        std::shared_ptr<scene2::SceneNode> node = build(key,json);
        node->doLayout();
        success = materialize(node, callback);
    } else {
        _loader->addTask([=,this](void) {
            this->enqueue(key);
            std::shared_ptr<JsonReader> reader = JsonReader::allocWithAsset(path);
            std::shared_ptr<JsonValue> json = (reader == nullptr ? nullptr : reader->readJson());
            std::shared_ptr<scene2::SceneNode> node = build(key,json);
            node->doLayout();
            Application::get()->schedule([=,this](void) {
                this->materialize(node,callback);
                return false;
            });
        });
    }
    
    return success;
}

/**
 * Internal method to support asset loading.
 *
 * This method supports either synchronous or asynchronous loading, as
 * specified by the given parameter.  If the loading is asynchronous,
 * the user may specify an optional callback function.
 *
 * This method is like the traditional read method except that it assumes
 * the JSON data has already been parsed.  The JSON tree should
 * be a tree of widget objects, where each widget object has the following
 * attribute values:
 *
 *      "type":     The node type (a Node or any subclass)
 *      "data":     Data (images, labels) that define the widget.  This
 *                  JSON object has a node-specific format.
 *      "format":   The layout manager to use for this Node. This layout
 *                  manager will apply to all the children (see below).
 *                  This JSON object has a layout-specific format.
 *      "layout":   Node placement using a the layout manager of the parent.
 *                  This is applied after parsing "data' and will override
 *                  any settings there. This JSON object has a
 *                  layout-specific format.
 *      "children": Any child Nodes of this one. This JSON object has a
 *                  named attribute for each child.
 *
 * With the exception of "type", all of these attributes are JSON objects.
 *
 * @param json      The directory entry for the asset
 * @param callback  An optional callback for asynchronous loading
 * @param async     Whether the asset was loaded asynchronously
 *
 * @return true if the asset was successfully loaded
 */
bool Scene2Loader::read(const std::shared_ptr<JsonValue>& json, LoaderCallback callback, bool async) {
    std::string key = json->key();
    if (_assets.find(key) != _assets.end() || _queue.find(key) != _queue.end()) {
        return false;
    }
    
    bool success = false;
    if (_loader == nullptr || !async) {
        enqueue(key);
        std::shared_ptr<scene2::SceneNode> node = build(key,json);
        node->doLayout();
        success = materialize(node,callback);
    } else {
        _loader->addTask([=,this](void) {
            this->enqueue(key);
            std::shared_ptr<scene2::SceneNode> node = build(key,json);
            node->doLayout();
            Application::get()->schedule([=,this](void) {
                this->materialize(node,callback);
                return false;
            });
        });
    }
    
    return success;
}

/**
 * Unloads the asset for the given directory entry
 *
 * An asset may still be available if it is referenced by a smart pointer.
 * See the description of the specific implementation for how assets
 * are released.
 *
 * This version of the method not only unloads the given {@link SceneNode},
 * but also any children attached to it in the JSON specification.
 *
 * @param json      The directory entry for the asset
 *
 * @return true if the asset was successfully unloaded
 */
bool Scene2Loader::purgeJson(const std::shared_ptr<JsonValue>& json) {
    bool success = purgeKey(json->key());
    if (json->has("children")) {
        auto kids = json->get("children");
        for(int ii = 0; ii < kids->size(); ii++) {
            success = purgeJson(kids->get(ii)) && success;
        }
    }
    return success;
}

/**
 * Attaches all generate nodes to the asset dictionary.
 *
 * As the asset dictionary must be updated in the main thread, we do
 * not update it until the entire node tree has been materialized. This
 * method assumes that each Node is named with its asset look-up key.
 *
 * @param key       The key to access the asset after loading
 * @param node      The scene asset
 *
 * @return true if the node was successfully attached
 */
bool Scene2Loader::attach(const std::string key, const std::shared_ptr<scene2::SceneNode>& node) {
    _assets[key] = node;
    bool success = true;
    for(int ii = 0; ii < node->getChildren().size(); ii++) {
        std::shared_ptr<scene2::SceneNode> item = node->getChild(ii);
        std::string local = key+"."+item->getName();
        success = attach(local, item) && success;
    }
    return success;
}



