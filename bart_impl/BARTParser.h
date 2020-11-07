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

class BARTSceneInfo {
public:
    BARTView mView;
    
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
    // exit on failure
    bool ParseFile(const char *path);
    
private:
    // underlying parsing functions
    bool parse_comment(FILE *scene);
    bool parse_viewport(FILE *scene);
    
private:
    BARTSceneInfo mSceneInfo;
    
};

}

#endif /* BARTParser_h */
