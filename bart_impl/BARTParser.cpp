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

}

