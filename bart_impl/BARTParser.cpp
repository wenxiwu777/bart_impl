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
    mObjCounter = 0;
    mObjID.clear();
    
    mSceneInfo.mMats.clear();
    
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
    mSceneInfo.mMats.clear();
    
    for (auto &item : mSceneInfo.mObjs) {
        delete item.second;
    }
    mSceneInfo.mObjs.clear();
    
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
    mSceneInfo.mObjs.insert(std::make_pair(mObjID, shpere));
    
    return true;
}

}