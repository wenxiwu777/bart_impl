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

#include "../includes/animation.h"
#include "../includes/raytracer_engine/MeshDesc.h"
#include "../includes/raytracer_engine/ResourcePool.h"
#include "../includes/raytracer_engine/Vec3.h"

namespace BART {

using LaplataRayTracer::MeshDesc;
using LaplataRayTracer::ResourcePool;
using LaplataRayTracer::Vec3f;
using LaplataRayTracer::TriFace;

struct BARTVec3 {
    float x;
    float y;
    float z;
};

struct BARTTexCoord {
    float u;
    float v;
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

struct BARTAmbientLight {
    BARTVec3 ambient_color;
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
        TEX_TRIANGLE,
        TEX_TRIANGLE_PATCH,
        TRIANGLE_PATCH,
        ANIMATED_TRIANGLE,
        MESH, };
    
public:
    int mMaterialID;
    
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

class BARTTexTriangle : public BARTShape {
public:
    string mTexName;
    BARTVec3 mVec[3];
    BARTTexCoord mTexCoord[3];
    
public:
    BARTTexTriangle() {
        mType = TEX_TRIANGLE;
    }
    
public:
    inline void SetVertex(int i, const BARTVec3& v) {
        mVec[i] = v;
    }
    inline void SetTexCoord(int i, const BARTTexCoord& texCoord) {
        mTexCoord[i] = texCoord;
    }
    
    inline const BARTVec3& GetVertex(int i) const {
        return mVec[i];
    }
    inline const BARTTexCoord& GetTexCoord(int i) const {
        return mTexCoord[i];
    }
    
};

class BARTTexTrianglePatch : public BARTTexTriangle {
public:
    BARTVec3 mNorm[3];
    
public:
    BARTTexTrianglePatch() {
        mType = TEX_TRIANGLE_PATCH;
    }
    
public:
    inline void SetNormal(int i, const BARTVec3& norm) {
        mNorm[i] = norm;
    }
    
    inline const BARTVec3& GetNormal(int i) const {
        return mNorm[i];
    }
    
};

class BARTTrianglePatch : public BARTShape {
public:
    BARTVec3 mVec[3];
    BARTVec3 mNorm[3];
    
public:
    BARTTrianglePatch() {
        mType = TRIANGLE_PATCH;
    }
    
public:
    inline void SetVertex(int i, const BARTVec3& v) {
        mVec[i] = v;
    }
    inline const BARTVec3& GetVertex(int i) const {
        return mVec[i];
    }

    inline void SetNormal(int i, const BARTVec3& norm) {
        mNorm[i] = norm;
    }
    inline const BARTVec3& GetNormal(int i) const {
        return mNorm[i];
    }
};

class BARTAnimatedTriangle : public BARTShape {
public:
    int mNumTimes;
    vector<float> mTimestamp;
    vector<BARTTrianglePatch> mTPs;
    
public:
    BARTAnimatedTriangle() {
        mType = ANIMATED_TRIANGLE;
        
        mNumTimes = 0;
        mTimestamp.clear();
        mTPs.clear();
    }
    
public:
    inline void AddOneTrianglePatch(float timestamp, const BARTTrianglePatch& tp) {
        mTimestamp.push_back(timestamp);
        mTPs.push_back(tp);
        
    }
    
};

class BARTMesh : public BARTShape {
public:
    // they will be used later in the setting up scene or generate scene phases.
    BARTVec3 mScale, mTranslate, mRotate;
    float mRotationAngle;
    
    int mMeshID;
    MeshDesc *mpMeshDesc;
    
public:
    BARTMesh() {
        mType = MESH;
        
    }
    
public:
    virtual void GenMeshDesc(int numTris, unsigned short *indices, int numVerts, int numNorms, int numTexs,
                             BARTVec3 *verts, BARTVec3 *normal, BARTTexCoord *texs) {
        mMeshID = ResourcePool::Instance()->AllocMesh();
        mpMeshDesc = ResourcePool::Instance()->QueryMesh(mMeshID);
        
        // fill it in
        if (verts != nullptr && numVerts > 0) {
            mpMeshDesc->mesh_vertices.reserve(numVerts);
            for (int i = 0; i < numVerts; ++i) {
                mpMeshDesc->mesh_vertices.push_back(Vec3f(verts[i].x, verts[i].y, verts[i].z));
            }
        }
        
        if (normal != nullptr && numNorms > 0) {
            mpMeshDesc->mesh_normal.reserve(numNorms);
            for (int i = 0; i < numNorms; ++i) {
                mpMeshDesc->mesh_normal.push_back(Vec3f(0.0f, 0.0f, 0.0f));
            }
        }
        
        if (texs != nullptr && numTexs > 0) {
            mpMeshDesc->mesh_texU.reserve(numTexs);
            mpMeshDesc->mesh_texV.reserve(numTexs);
            for (int u = 0; u < numTexs; ++u) {
                mpMeshDesc->mesh_texU.push_back(0.0f);
            }
            for (int v = 0; v < numTexs; ++v) {
                mpMeshDesc->mesh_texV.push_back(0.0f);
            }
        }
        
        if (indices != nullptr && numTris > 0) {
            if (normal == nullptr) {
                mpMeshDesc->mesh_vertex_faces.reserve(numVerts);
                for (int i = 0; i < numVerts; ++i) {
                    mpMeshDesc->mesh_vertex_faces.push_back(vector<int>());
                }
            }
            
            // all indices are based on zero(aero-based)
            int idx_t0, idx_t1, idx_t2;
            int idx_n0, idx_n1, idx_n2;
            int idx_v0, idx_v1, idx_v2;
            
            int idx = 0;
            for (int face_idx = 0; face_idx < numTris; ++face_idx) {
                if (texs != nullptr) {
                    idx_t0 = indices[idx++];
                    idx_t1 = indices[idx++];
                    idx_t2 = indices[idx++];
                }
                if (normal != nullptr) {
                    idx_n0 = indices[idx++];
                    idx_n1 = indices[idx++];
                    idx_n2 = indices[idx++];
                }
                idx_v0 = indices[idx++];
                idx_v1 = indices[idx++];
                idx_v2 = indices[idx++];
                
                TriFace face = {idx_v0, idx_v1, idx_v2};
                mpMeshDesc->mesh_face_datas.push_back(face);
                
                if (normal == nullptr) {
                    // sice the normal info is not given,
                    // so we need to set up the vertex-face-list
                    // so that we can calculate per-vertex normal later on.
                    // all three indices point to the same face id.
                    mpMeshDesc->mesh_vertex_faces[idx_v0].push_back(face_idx);
                    mpMeshDesc->mesh_vertex_faces[idx_v1].push_back(face_idx);
                    mpMeshDesc->mesh_vertex_faces[idx_v2].push_back(face_idx);
                }
                else {
                    // access the normal vector by normal index
                    mpMeshDesc->mesh_normal[idx_v0].v[0] = normal[idx_n0].x;
                    mpMeshDesc->mesh_normal[idx_v0].v[1] = normal[idx_n0].y;
                    mpMeshDesc->mesh_normal[idx_v0].v[2] = normal[idx_n0].z;
                    
                    mpMeshDesc->mesh_normal[idx_v1].v[0] = normal[idx_n1].x;
                    mpMeshDesc->mesh_normal[idx_v1].v[1] = normal[idx_n1].y;
                    mpMeshDesc->mesh_normal[idx_v1].v[2] = normal[idx_n1].z;
                    
                    mpMeshDesc->mesh_normal[idx_v2].v[0] = normal[idx_n2].x;
                    mpMeshDesc->mesh_normal[idx_v2].v[1] = normal[idx_n2].y;
                    mpMeshDesc->mesh_normal[idx_v2].v[2] = normal[idx_n2].z;
                    
                }
    
                // finally, we deal with texture u and v
                if (texs != nullptr) {
                    mpMeshDesc->mesh_texU[idx_v0] = texs[idx_t0].u;
                    mpMeshDesc->mesh_texU[idx_v1] = texs[idx_t1].u;
                    mpMeshDesc->mesh_texU[idx_v2] = texs[idx_t2].u;
                    
                    mpMeshDesc->mesh_texV[idx_v0] = texs[idx_t0].v;
                    mpMeshDesc->mesh_texV[idx_v1] = texs[idx_t1].v;
                    mpMeshDesc->mesh_texV[idx_v2] = texs[idx_t2].v;
                }
                
            }
        }
        
        mpMeshDesc->mesh_vertex_count = numVerts;
        mpMeshDesc->mesh_face_count = numTris;
        mpMeshDesc->mesh_support_uv = (texs != nullptr);
    }
    
};

//
class BARTSceneInfo {
public:
    BARTView mView;
    BARTBackground mBack;
    BARTAmbientLight mAmbient;
    
    vector<BARTLight> mLights; // hold all lights in the scene
    
    map<int, BARTMaterial> mMats; // hold all materials in the scene
    
    map<string, BARTShape *> mObjs; // hold all objects in the scene
    
public:
    BARTSceneInfo();
    ~BARTSceneInfo();

};

//
struct AnimFrameInfo {
    float start_time;
    float end_time;
    int num_frames;
    
};

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
    bool parse_triangle_series(FILE *scene);
    bool parse_XForm(FILE *scene);
    bool parse_AmbientOrAnimParams(FILE *scene);
    bool parse_KFrames(FILE *scene); // 解析 k x 指令配对的关键帧列表 parse the list of key frames taged with k-x pair.
    bool parse_mesh(FILE *scene);
    
private:
    bool parse_non_anim_triangle(FILE *scene);
    bool parse_anim_triangle(FILE *scene);
    
    bool read_vectors(FILE *scene, const char *type, int *numVecs, BARTVec3 **vecs);
    bool read_textures(FILE *scene, char *textureName, int *numTexs, BARTTexCoord **texs);
    bool read_triangles(FILE *scene, int *num_tris, unsigned short **indices,
                        BARTVec3 *verts, BARTVec3 *norms, BARTTexCoord *texs);
    
    void eat_white_space(FILE *scene);
    
    void end_parse_xform(void);
    
    string path_goto_forward(const string& path, const string& fpath) const;
    string path_goto_backward(const string& path) const;
    string gen_object_id(void);
    
private:
    void cleanup();
    void reset_RTS_vectors();
    
private:
    int mMaterialIndex; // it may be used as material id later probably, cause material is attached to object
    int mObjIndex;
    // the followings are used as global vars, but only take effect to XS(static form),
    // i guess the reason is that static form means a mesh object which has its initial coordinate, so we need to
    // set r/t/s vectors for it as a whole, while single geometric object we can represent by given its inital coordinate.
    // Very important, when we enter into XS instruction, we assign these vars, then any mesh object is parsed in this scope will share
    // these vars, afterwards then, these vars will be reset, this is because mesh can be not attached to any XS tag, in other words,
    // mesh can be independent to any tag but it is accossiated with material and r/t/s vectors.
    BARTVec3 mScale, mTranslate, mRotate;
    float mRotationAngle;
    
    string mObjID;
    BARTSceneInfo mSceneInfo;
    int mDetailLevel;
    
    AnimFrameInfo mAnimFrameInfo;
    
    AnimationList *mpAnimList;
    
};

}

#endif /* BARTParser_h */
