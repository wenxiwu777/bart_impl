//
//  BARTParser.cpp
//  bart_impl
//
//  Created by 吴文希 on 2020/11/7.
//  Copyright © 2020 wwx. All rights reserved.
//

#include "BARTParser.h"

namespace BART {

////
BARTSceneInfo::BARTSceneInfo() {
    
}

BARTSceneInfo::~BARTSceneInfo() {
    
}

//


////
BARTParser::BARTParser() {
    
}

BARTParser::~BARTParser() {
    
}

//
void BARTParser::InitParser(void) {
    reset_RTS_vectors();
    
    mMaterialIndex = 0;
    mDetailLevel = 0;
    
    cleanup();
    
}

bool BARTParser::ParseFile(const char *path) {
    FILE *scene = fopen(path, "r");
    if (!scene) {
        return false;
    }
    
    int ch;
    bool failed = false;
    while ((ch = getc(scene)) != EOF) {
        switch (ch) {
        // eat white character
        case ' ':
        case '\t':
        case '\n':
        case '\f':
        case '\r':
            continue;
        //
        case '#':
        case '%':
            failed = parse_comment(scene);
            break;
        //
        case 'v':
            failed = parse_viewport(scene);
            break;
        //
        case 'l':
            failed = parse_light(scene);
            break;
        //
        case 'b':
            failed = parse_background(scene);
            break;
        //
        case 'f':
            failed = parse_material(scene);
            break;
        //
        case 'c':
            failed = parse_cone(scene);
            break;
        //
        case 's':
            failed = parse_sphere(scene);
            break;
        //
        case 'p':
            failed = parse_polygon(scene);
            break;
        //
        case 'i':
            failed = parse_include(scene);
            break;
        //
        case 'd':
            failed = parse_detail_level(scene);
            break;
        //
        case 't':
            failed = parse_triangle_series(scene);
            break;
        //
        case 'x':
            failed = parse_XForm(scene);
            break;
        case '}':
            end_parse_xform();
            break;
        default:
            printf("reached unknown AFF instruction\n");
            failed = true;
            break;
        }
        
        //
        if (failed) {
            printf("error occured while paring, about to exit!\n");
            break;
        }
    }
    
    fclose(scene);
    return failed;
}

void BARTParser::DoneParser(void) {
    cleanup();
    
}

BARTSceneInfo& BARTParser::GetSceneInfo() {
    return mSceneInfo;
}

//
bool BARTParser::parse_comment(FILE *scene) {
    char str[512];
    memset(str, 0, sizeof(str));
    fgets(str, 1000, scene);
    return true;
}

bool BARTParser::parse_viewport(FILE *scene) {
    BARTVec3 from;
    BARTVec3 at;
    BARTVec3 up;
    float fov;
    float hither;
    int resx;
    int resy;
       
    if(fscanf(scene, " from %f %f %f", &from.x, &from.y, &from.z) != 3) {
        printf("Parser view member(from) syntax error\n");
        return false;
    }
  
    if(fscanf(scene, " at %f %f %f", &at.x, &at.y, &at.z) != 3) {
        printf("Parser view member(at) syntax error\n");
        return false;
    }
  
    if(fscanf(scene, " up %f %f %f", &up.x, &up.y, &up.z) != 3) {
        printf("Parser view member(up) syntax error\n");
        return false;
    }

    if(fscanf(scene, " angle %f", &fov) != 1) {
        printf("Parser view member(angle) syntax error\n");
        return false;
    }
        
  
    if(fscanf(scene, " hither %f", &hither) != 1) {
        printf("Parser view member(hither) syntax error\n");
        return false;
    }
     
    if(hither < 0.0001) {
        hither = 1.0f;
    }
     
    if(fscanf(scene, " resolution %d %d", &resx, &resy) != 2) {
        printf("Parser view member(resolution) syntax error\n");
        return false;
    }
    
    // since we succeded to parse the view member, we set it up now.
    mSceneInfo.mView = {from, at, up, fov, hither, resx, resy};
    
    return true;
}

bool BARTParser::parse_light(FILE *scene) {
    BARTVec3 pos;
    BARTVec3 col;
    int num = 0;
    int is_animated;
    char name[100];
    memset(name, 0, sizeof(name));

    is_animated = getc(scene);
    if (is_animated != 'a') {
        ungetc(is_animated, scene);
        is_animated = 0;
    }

    if (is_animated) {
        fscanf(scene, "%s", name);
    }

    if (fscanf(scene, "%f %f %f ",&pos.x, &pos.y, &pos.z) != 3) {
        printf("Light source position syntax error\n");
        return false;
    }

    num = fscanf(scene, "%f %f %f ",&col.x, &col.y, &col.z);
    if (num == 0) {
        col = {1.0f, 1.0f, 1.0f};
    }
    else if (num != 3) {
        printf("Light source color syntax error\n");
        return false;
    }
    
    mSceneInfo.mLight = { pos, col, is_animated, name };
    
    return true;
}

bool BARTParser::parse_background(FILE *scene) {
    BARTVec3 bgcolor;
        
    if(fscanf(scene, "%f %f %f",&bgcolor.x, &bgcolor.y, &bgcolor.z) != 3) {
        printf("background color syntax error\n");
        return false;
    }
    
    mSceneInfo.mBack.bgcolor = bgcolor;
    
    return true;
}

bool BARTParser::parse_material(FILE *scene) {
    float kd, ks, phong_pow, t, ior;
    BARTVec3 col;
    int extened_params;

    extened_params = getc(scene);
    if(extened_params != 'm') {
        ungetc(extened_params, scene);
        extened_params = 0;
    }

    if(extened_params) {
        BARTVec3 amb, diff, spec;
        
        if(fscanf(scene, "%f %f %f", &amb.x, &amb.y, &amb.z) != 3) {
            printf("fill material ambient syntax error\n");
            return false;
        }
        
        if(fscanf(scene, "%f %f %f", &diff.x, &diff.y, &diff.z) != 3) {
            printf("fill material diffuse syntax error\n");
            return false;
        }
        
        if(fscanf(scene,"%f %f %f", &spec.x, &spec.y, &spec.z) != 3) {
            printf("fill material specular syntax error\n");
            return false;
        }
        
        if (fscanf(scene, "%f %f %f", &phong_pow, &t, &ior) != 3) {
            printf("fill material (phong, transp, IOR) syntax error\n");
            return false;
        }
        
        // add it to material storage
        BARTMaterial mat;
        mat.amb = amb;
        mat.diff = diff;
        mat.spec = spec;
        mat.shine = phong_pow;
        mat.T = t;
        mat.IOR = ior;
        mSceneInfo.mMats.insert(std::make_pair(mObjID, mat));
           
    } else {
        if (fscanf(scene, "%f %f %f", &col.x, &col.y, &col.z) != 3) {
            printf("fill color syntax error\n");
            return false;
        }
       
        if (fscanf(scene, "%f %f %f %f %f", &kd, &ks, &phong_pow, &t, &ior) != 5) {
            printf("fill material syntax error");
            return false;
        }
        
        // add it to material storage
        BARTMaterial mat;
        mat.amb = col;
        mat.diff = { kd * mat.amb.x, kd * mat.amb.y, kd * mat.amb.z };
        mat.spec = { ks * mat.amb.x, ks * mat.amb.y, ks * mat.amb.z };
        mat.shine = phong_pow;
        mat.T = t;
        mat.IOR = ior;
        mSceneInfo.mMats.insert(std::make_pair(mObjID, mat));

    }
    
    return true;
}

bool BARTParser::parse_cone(FILE *scene) {
    BARTVec3 base_pt;
    BARTVec3 apex_pt;
    float r0, r1;
    
    if(fscanf(scene, " %f %f %f %f %f %f %f %f", &base_pt.x, &base_pt.y, &base_pt.z, &r0,
             &apex_pt.x, &apex_pt.y, &apex_pt.z, &r1) !=8 ) {
        printf("cylinder or cone syntax error\n");
        return false;
    }
    
    if(r0 < 0.0) {
       r0 = -r0;
       r1 = -r1;
    }
    
    //
    BARTCone *cone = new BARTCone;
    cone->base_pt = base_pt;
    cone->apex_pt = apex_pt;
    cone->r0 = r0;
    cone->r1 = r1;
    cone->mMaterialID = mMaterialIndex;
    mSceneInfo.mObjs.insert(std::make_pair(mObjID, cone));
    
    return true;
}

bool BARTParser::parse_sphere(FILE *scene) {
    float radius;
    BARTVec3 center;
        
    if(fscanf(scene, "%f %f %f %f", &center.x, &center.y, &center.z, &radius) != 4) {
        printf("sphere syntax error\n");
        return false;
    }
    
    //
    BARTSphere *shpere = new BARTSphere;
    shpere->center = center;
    shpere->radius = radius;
    shpere->mMaterialID = mMaterialIndex;
    mSceneInfo.mObjs.insert(std::make_pair(mObjID, shpere));
    
    return true;
}

bool BARTParser::parse_polygon(FILE *scene) {
    int is_patch;
    int nverts;
    int q;
    
    is_patch = getc(scene);
    if (is_patch != 'p') {
        ungetc(is_patch, scene);
        is_patch = 0;
    }
   
    if (fscanf(scene, "%d", &nverts) != 1) {
        printf("polygon or patch syntax error\n");
        return false;
    }
    
    if (is_patch) {
        BARTPolygonPatch *poly_path = new BARTPolygonPatch;
        for (q = 0; q < nverts; q++) {
            BARTPolygonPatch::Patch patch;
            
            if(fscanf(scene, " %f %f %f", &patch.pt.x, &patch.pt.y, &patch.pt.z) != 3) {
                printf("polygon patch(v) syntax error\n");
                return false;
            }
           
            if(fscanf(scene, " %f %f %f", &patch.norm.x, &patch.norm.y, &patch.norm.z) != 3) {
                printf("polygon patch(n) syntax error\n");
                return false;
            }
            
            poly_path->Add(patch);
        }
        poly_path->mMaterialID = mMaterialIndex;
        mSceneInfo.mObjs.insert(std::make_pair(mObjID, poly_path));
    } else {
        BARTPolygon *poly = new BARTPolygon;
        for (q = 0; q < nverts; q++) {
            BARTVec3 vec;
            
            if(fscanf(scene, " %f %f %f", &vec.x, &vec.y, &vec.z) != 3) {
                printf("polygon syntax error\n");
                return false;
            }
            
            poly->Add(vec);
        }
        poly->mMaterialID = mMaterialIndex;
        mSceneInfo.mObjs.insert(std::make_pair(mObjID, poly));
    }
    
    return true;
}

bool BARTParser::parse_include(FILE *scene) {
    char filename[100];
    memset(filename, 0, sizeof(filename));
    int detail_level;
    
    if (fscanf(scene, "%d %s", &detail_level, filename) != 2) {
        printf("Error: could not parse include.\n");
        return false;
    }

    bool failed = false;
    if (detail_level <= mDetailLevel) {
        failed = ParseFile(filename);  // parse the file recursively
    } else {
        printf("Skipping include file: %s\n",filename);
    }
    
    return failed;
}

bool BARTParser::parse_detail_level(FILE *scene) {
    if (fscanf(scene, "%d", &mDetailLevel) != 1) {
        printf("Error: could not parse detail level.\n");
        return false;
    }
    
    return true;
}

bool BARTParser::parse_triangle_series(FILE *scene) {
    bool result = true;
    int tri_tag = getc(scene);
    if (tri_tag == 't') { // tt
        result = parse_non_anim_triangle(scene);
        
    } else if (tri_tag == 'p') {
        tri_tag = getc(scene);
        if (tri_tag == 'a') { // tpa
            result = parse_anim_triangle(scene);
            
        } else {
            printf("Error: could not parse triangle series\n");
            result = false;
        }
    } else {
        printf("Error: could not parse triangle series\n");
        result = false;
    }
    
    return result;
}

bool BARTParser::parse_XForm(FILE *scene) {
    char name[100];
    char ch;
    int is_static;
    
    memset(name, 0, sizeof(name));

    is_static = getc(scene);
    if (is_static != 's') {
       ungetc(is_static, scene);
       is_static=0;
    }

    if (is_static) {
        if (fscanf(scene, " %f %f %f %f %f %f %f %f %f %f",
                   &mScale.x, &mScale.y, &mScale.z,
                   &mRotate.x, &mRotate.y, &mRotate.z, &mRotationAngle,
                   &mTranslate.x, &mTranslate.y, &mTranslate.z) !=10 ) {
            printf("Error: could not read static transform.\n");
            return false;
       }
        
       this->eat_white_space(scene);
        
       ch = getc(scene);
       if (ch != '{') {
           printf("Error: { expected when parsing xs form.\n");
           return false;
       }

      // the actual object is mesh, it will be added to scene info when it parses 'm' instruction.
      // so we won't add anything herein.
       
    } else {
       if (fscanf(scene, "%s", name) != 1) {
           printf("Error: could not read transform name when parsing x form.\n");
           return false;
       }
        
       this->eat_white_space(scene);
        
       ch = getc(scene);
       if (ch != '{') {
           printf("Error: { expected when parsing x from.\n");
           return false;
       }
       
       // we wont' add anything herein, the reason is as the above.
    }
    
    return true;
}

//
bool BARTParser::parse_non_anim_triangle(FILE *scene) {
    int is_patch;
    int q;
    char texturename[100];
    memset(texturename, 0, sizeof(texturename));
    
    is_patch = getc(scene);
    if (is_patch != 'p') {
        ungetc(is_patch, scene);
        is_patch = 0;
    } // !ttp
        
    fscanf(scene, "%s", texturename);
    
    bool result = true;
    if (is_patch) {
        BARTTexTrianglePatch *ttp = new BARTTexTrianglePatch;
        ttp->mTexName = (char *)texturename;
        
        for (q = 0; q < 3; ++q) {
            if (fscanf(scene, " %f %f %f", &(ttp->mVec[q].x), &(ttp->mVec[q].y), &(ttp->mVec[q].z)) != 3) {
                result = false;
                break;
            }
            
            if (fscanf(scene, " %f %f %f", &(ttp->mNorm[q].x), &(ttp->mNorm[q].y), &(ttp->mNorm[q].z)) !=3) {
                result = false;
                break;
            }
            
            if (fscanf(scene, " %f %f ", &(ttp->mTexCoord[q].u), &(ttp->mTexCoord[q].v)) != 2) {
                result = false;
                break;
            }
        }
        
        if (result) {
            ttp->mMaterialID = mMaterialIndex;
            mSceneInfo.mObjs.insert(std::make_pair(mObjID, ttp));
        }
    } else {
        BARTTexTriangle *tt = new BARTTexTriangle;
        tt->mTexName = (char *)texturename;
        
        for (q = 0; q < 3; ++q) {
            if (fscanf(scene, " %f %f %f", &(tt->mVec[q].x), &(tt->mVec[q].y), &(tt->mVec[q].z)) != 3) {
                result = false;
                break;
            }
            
            if (fscanf(scene, " %f %f ", &(tt->mTexCoord[q].u), &(tt->mTexCoord[q].v)) != 2) {
                result = false;
                break;
            }
        }
        
        if (result) {
            tt->mMaterialID = mMaterialIndex;
            mSceneInfo.mObjs.insert(std::make_pair(mObjID, tt));
        }
    }
    
    if (!result) {
        printf("Error: could not parse textured triangle data(may be vertex/normal/uv)\n");
    }
    
    return result;
}

bool BARTParser::parse_anim_triangle(FILE *scene) {
    int q, w;
    int num_times;
    
    fscanf(scene, "%d", &num_times);
    
    BARTAnimatedTriangle *tpa = new BARTAnimatedTriangle;
    tpa->mNumTimes = num_times;
    
    for (q = 0; q < num_times; ++q) {
        float timestamp;
        if (fscanf(scene, " %f", &timestamp) != 1) {
            printf("Error: could not parse animated triangle(timestamp)\n");
            return false;
        }
        
        BARTTrianglePatch tp;
        for (w = 0; w < 3; ++w) {
            if (fscanf(scene, " %f %f %f", &(tp.mVec[w].x), &(tp.mVec[w].y), &(tp.mVec[w].z)) != 3) {
                printf("Error: could not parse animated triangle(vertex)\n");
                return false;
            }
            
            if (fscanf(scene, " %f %f %f", &(tp.mNorm[w].x), &(tp.mNorm[w].y), &(tp.mNorm[w].z)) != 3) {
                printf("Error: could not parse animated triangle(normal)\n");
                return false;
            }
        }
        tpa->AddOneTrianglePatch(timestamp, tp);
    }
    
    tpa->mMaterialID = mMaterialIndex;
    mSceneInfo.mObjs.insert(std::make_pair(mObjID, tpa));
    
    return true;
}

//
void BARTParser::eat_white_space(FILE *scene) {
    char ch=getc(scene);
    while (ch==' ' || ch=='\t' || ch=='\n' || ch=='\f' || ch=='\r') {
        ch=getc(scene);
    }
    ungetc(ch, scene);
}

void BARTParser::end_parse_xform(void) {
    // reset r/t/s vectors
    this->reset_RTS_vectors();
    
}

//
void BARTParser::cleanup() {
    mObjID.clear();
    
    mSceneInfo.mMats.clear();
        
    for (auto &item : mSceneInfo.mObjs) {
        delete item.second;
    }
    mSceneInfo.mObjs.clear();
}

void BARTParser::reset_RTS_vectors() {
    mScale = {1.0f, 1.0f, 1.0f};
    mRotate = {1.0f, 1.0f, 1.0f};
    mTranslate = {1.0f, 1.0f, 1.0f};
    mRotationAngle = 0.0f;
}

}
