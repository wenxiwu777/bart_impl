//
//  BARTParser.h
//  bart_impl
//
//  Created by 吴文希 on 2020/11/7.
//  Copyright © 2020 wwx. All rights reserved.
//

#ifndef BARTParser_h
#define BARTParser_h

#include <stdio.h>
#include <string.h>

#include <vector>
#include <string>
#include <map>

using std::vector;
using std::string;
using std::map;

namespace BART {

struct BARTVec3 {
    float x;
    float y;
    float z;
};

struct BARTView {
    BARTVec3 from;
    BARTVec3 at;
    BARTVec3 up;
    float angle;
    float hither;
    int xres;
    int yres;
    
};

struct BARTLight {
    BARTVec3 pos;
    BARTVec3 col;
    int is_animated;
    string name;
    
};

struct BARTBackground {
    BARTVec3 bgcolor;
    
};

struct BARTMaterial {
    BARTVec3 amb;
    BARTVec3 diff;
    BARTVec3 spec;
    float shine;
    float T;
    float IOR; // index of refraction
};

class BARTShape {
public:
    enum Type {
        UNKNOWN = -1,
        CONE = 0,
        SHPERE,
        POLY,
        POLY_PATCH,
        MESH, };
    
public:
    BARTShape() {
        mType = UNKNOWN;
    }
    
public:
    inline Type GetType() const {
        return mType;
    }
    
protected:
    Type mType; // only can be set by its sub-classes, but can be read via GetType() from outer
    
};

class BARTCone : public BARTShape {
public:
    BARTVec3 base_pt;
    BARTVec3 apex_pt;
    float r0;
    float r1;
    
public:
    BARTCone() {
        mType = BARTShape::CONE;
    }
    
};

class BARTSphere : public BARTShape {
public:
    BARTVec3 center;
    float radius;
    
public:
    BARTSphere() {
        mType = BARTShape::SHPERE;
    }
    
};

class BARTPolygon : public BARTShape {
public:
    vector<BARTVec3> mPts;
    
public:
    BARTPolygon() {
        mType = POLY;
    }

public:
    inline size_t Count() const {
        return mPts.size();
    }
    
    inline const BARTVec3& At(int index) const {
        return mPts[index];
    }
    
    inline void Add(const BARTVec3& vec) {
        mPts.push_back(vec);
    }

};

class BARTPolygonPatch : public BARTShape {
public:
    struct Patch {
        BARTVec3 pt;
        BARTVec3 norm;
    };
    
public:
    vector<Patch> mPatches;
    
public:
    BARTPolygonPatch() {
        mType = POLY_PATCH;
    }
    
public:
    inline size_t Count() const {
        return mPatches.size();
    }
    
    inline const BARTPolygonPatch::Patch& At(int index) const {
        return mPatches[index];
    }
    
    inline void Add(const BARTPolygonPatch::Patch& patch) {
        mPatches.push_back(patch);
    }
    
};

//
class BARTSceneInfo {
public:
    BARTView mView;
    BARTLight mLight;
    BARTBackground mBack;
    
    map<string, BARTMaterial> mMats; // hold all materials in the scene
    
    map<string, BARTShape *> mObjs; // hold all objects in the scene
    
public:
    BARTSceneInfo();
    ~BARTSceneInfo();

};

//
class BARTParser {
public:
    BARTParser();
    ~BARTParser();
    
public:
    void InitParser(void);
    
    // exit on failure
    bool ParseFile(const char *path);
    
    void DoneParser(void);
    
public:
    BARTSceneInfo& GetSceneInfo();
    
private:
    // underlying parser's functions
    bool parse_comment(FILE *scene);
    bool parse_viewport(FILE *scene);
    bool parse_light(FILE *scene);
    bool parse_background(FILE *scene);
    bool parse_material(FILE *scene); //
    bool parse_cone(FILE *scene);
    bool parse_sphere(FILE *scene);
    bool parse_polygon(FILE *scene);
    bool parse_include(FILE *scene);
    bool parse_detail_level(FILE *scene);
    
private:
    void cleanup();
    
private:
    int mObjCounter;
    string mObjID;
    BARTSceneInfo mSceneInfo;
    int mDetailLevel;
    
};

}

#endif /* BARTParser_h */
