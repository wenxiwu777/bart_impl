//
//  BARTParser.cpp
//  bart_impl
//
//  Created by 吴文希 on 2020/11/7.
//  Copyright © 2020 wwx. All rights reserved.
//

//#include "../includes/animation.h"

#include "BARTParser.h"

#ifndef M_PI
#define M_PI 3.141593
#endif // M_PI

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
    mObjIndex = 0;
    mDetailLevel = 0;
    
    cleanup();
    
    mpAnimList = nullptr;
}

bool BARTParser::ParseFile(const char *path) {
    // set up our navigator, the global path
    mObjID = path_goto_forward(mObjID, path);
    
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
        case 'a':
            failed = parse_AmbientOrAnimParams(scene);
            break;
        case 'k':
            failed = parse_KFrames(scene);
            break;
        case 'm':
            failed = parse_mesh(scene);
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
    
    mObjID = path_goto_backward(mObjID);
    
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
    
    BARTLight light = { pos, col, is_animated, name };
    mSceneInfo.mLights.push_back(light);
    
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
        mSceneInfo.mMats.insert(std::make_pair(++mMaterialIndex, mat));
           
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
        mSceneInfo.mMats.insert(std::make_pair(++mMaterialIndex, mat));

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
    
    mSceneInfo.mObjs.insert(std::make_pair(gen_object_id(), cone));
    
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
    
    mSceneInfo.mObjs.insert(std::make_pair(gen_object_id(), shpere));
    
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
        mSceneInfo.mObjs.insert(std::make_pair(gen_object_id(), poly_path));
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
        mSceneInfo.mObjs.insert(std::make_pair(gen_object_id(), poly));
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
        // but we still need to deep the path.
        mObjID = path_goto_forward(mObjID, "x_x");
       
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
        string sub_path = "xs_";
        string str_name = (const char *)name;
        sub_path += str_name;
        mObjID = path_goto_forward(mObjID, sub_path);

    }
    
    return true;
}

bool BARTParser::parse_AmbientOrAnimParams(FILE *scene) {
//    char ch;
    int is_ambient;
    
    is_ambient = getc(scene);
    
    if (is_ambient != 'm') {
        ungetc(is_ambient, scene);
        is_ambient = 0;
    }
    
    if (is_ambient) {
        BARTVec3 amb;
        if (fscanf(scene, "%f %f %f", &amb.x, &amb.y, &amb.z) !=3 ) {
            printf("Error: could not parse ambient light (am).\n");
            return false;
        }
        
        mSceneInfo.mAmbient.ambient_color = amb;
    } else {
        if (fscanf(scene, "%f %f %d", &mAnimFrameInfo.start_time, &mAnimFrameInfo.end_time, &mAnimFrameInfo.num_frames) !=3 ) {
            printf("Error: could not parse animations parameters.\n");
            return false;
            
        }
    }
    
    return true;
}

bool BARTParser::parse_KFrames(FILE *scene) {
    char name[256];
    char motion[256];
    char ch;
    int visibility;
    int  ret, i, key_frame_number;
    float time, x, y, z, angle, te, co, bi;
    PositionKey *pos_keys;
    RotationKey *rot_keys;
    Animation *animation;
    struct AnimationList *animationlist;
    
    memset(name, 0, sizeof(name));
    memset(motion, 0, sizeof(motion));
    
    if (fscanf(scene, "%s", name) !=1) {
        printf("Error: could not read name of animation.\n");
        return false;
    }
    
    this->eat_white_space(scene);
    
    ch = getc(scene);
    
    if (ch != '{') {
        printf("Error: syntax error, could not find a { in animation %s.\n", name);
        return false;
    }
    
    animationlist = (struct AnimationList*)calloc(1, sizeof(struct AnimationList));
    if (!animationlist) {
        printf("Error: failed to allocate animation list!\n");
        return false;
    }
    
    animationlist->next = mpAnimList;
    mpAnimList = animationlist;
    animation = &(animationlist->animation);
    
    animation->name = (char *)malloc(sizeof(name));
    strcpy(animation->name, name);
    
    animation->translations = nullptr;
    animation->rotations = nullptr;
    animation->scales = nullptr;
    animation->visibilities = nullptr;
    
    this->eat_white_space(scene);
    
    while ((ch = getc(scene)) != '}') {
        ungetc(ch, scene);
        
        if (fscanf(scene, "%s %d", motion, &key_frame_number) != 2) {
            printf("Error: could not read name of motion or number of keyframes for animation.\n");
            return false;
        }
        
        if (key_frame_number < 4 && strcmp("visibility", (const char *)motion)) {
            printf("Error: there must be at least 4 keyframes for %s.\n", name);
            return false;
        }
        
        if (strcmp((const char *)motion, "transl") == 0
            || strcmp((const char *)motion, "scale") == 0) {
            pos_keys = (PositionKey*)calloc(key_frame_number, sizeof(PositionKey));
            for (i = 0; i < key_frame_number; ++i) {
                ret = fscanf(scene, " %f %f %f %f %f %f %f", &time, &x, &y, &z, &te, &co, &bi);
                if(ret != 7) {
                    printf("error in parsing translation or scale keyframes for %s\n", animation->name);
                    return false;
                }
                pos_keys[i].t = time;
                pos_keys[i].P.x = x;
                pos_keys[i].P.y = y;
                pos_keys[i].P.z = z;
                pos_keys[i].tension = te;
                pos_keys[i].continuity = co;
                pos_keys[i].bias = bi;
            }
            if (strcmp((const char *)motion, "transl") == 0) {
                animation->translations = KB_PosInitialize(key_frame_number, pos_keys);
            }
            else {
                animation->scales = KB_PosInitialize(key_frame_number, pos_keys);
            }
            free(pos_keys);
        }
        else if (strcmp((const char *)motion, "rot") == 0) {
            rot_keys = (RotationKey*)calloc(key_frame_number, sizeof(RotationKey));
            for (i=0; i < key_frame_number; ++i) {
                ret = fscanf(scene," %f %f %f %f %f %f %f %f", &time, &x, &y, &z, &angle, &te, &co, &bi);
                if(ret != 8) {
                    printf("error in parsing rotation keyframes for %s\n", animation->name);
                    return false;
                }
                rot_keys[i].t = time;
                rot_keys[i].Rot.x = x;
                rot_keys[i].Rot.y = y;
                rot_keys[i].Rot.z = z;
                rot_keys[i].Rot.angle = angle*M_PI/180.0;
                rot_keys[i].tension = te;
                rot_keys[i].continuity = co;
                rot_keys[i].bias = bi;
            }
            animation->rotations = KB_RotInitialize(key_frame_number, rot_keys);
            free(rot_keys);
        }
        else if (strcmp((const char *)motion, "visibility") == 0) {
            VisKey *vis_keys = (VisKey*)calloc(key_frame_number, sizeof(VisKey));
            for (i=0; i < key_frame_number; ++i) {
                ret = fscanf(scene, " %f %d", &time, &visibility);
                if(ret != 2) {
                    printf("error in parsing visibility keyframes for %s\n", animation->name);
                    return false;
                }
                vis_keys[i].time = time;
                vis_keys[i].visibility = visibility;
             }
             animation->visibilities = vis_keys;
             animation->numVisibilities += key_frame_number;
        }
        else {
            printf("Error: unknown keyframe type (%s). Must be transl, rot, or scale.\n", motion);
            return false;
        }
        
        this->eat_white_space(scene);
    }
    
    return true;
}

bool BARTParser::parse_mesh(FILE *scene) {
    char strid[256];
    int num_verts,num_norms,num_texs,num_tris;
    BARTVec3 *verts = nullptr;
    BARTVec3 *norms = nullptr;
    BARTTexCoord *texs = nullptr;
    unsigned short *indices;
    char tex_name[256];
    
    memset(strid, 0, sizeof(strid));
    memset(tex_name, 0, sizeof(tex_name));
    
    if (fscanf(scene, "%s", strid) != 1) {
        printf("Error: could not parse mesh (could not find 'vertices').\n");
        return false;
    }
    
    if (strcmp((const char *)strid, "vertices")) {
        printf("Error: could not parse mesh (expected 'vertices').\n");
        return false;
    }
    
    //getVectors(fp,"vertices",&num_verts,&verts);
    if (!read_vectors(scene, "vertices", &num_verts, &verts)) {
        return false;
    }

    fscanf(scene, "%s", strid);
    if (!strcmp((const char *)strid, "normals")) {
        //getVectors(fp,"normals",&num_norms,&norms);
        if (!read_vectors(scene, "normals", &num_norms, &norms)) {
            return false;
        }
        fscanf(scene, "%s", strid);
    }
    
    if (!strcmp((const char *)strid, "texturecoords")) {
        //getTextureCoords(fp,texturename,&num_texs,&txts);
        if (!read_textures(scene, tex_name, &num_texs, &texs)) {
            return false;
        }
        fscanf(scene, "%s", strid);
    }
    
    if (!strcmp((const char *)strid, "triangles")) {
        //getTriangles(fp,&num_tris,&indices,verts,norms,txts);
        if (!read_triangles(scene, &num_tris, &indices, verts, norms, texs)) {
            return false;
        }
    }
    else {
        printf("Error: expected 'triangles' in mesh.\n");
        return false;
    }
    
    // push it to object lits
    BARTMesh *mesh = new BARTMesh;
    mesh->mRotate = mRotate;
    mesh->mScale = mScale;
    mesh->mTranslate = mTranslate;
    mesh->mRotationAngle = mRotationAngle;
    mesh->mMaterialID = mMaterialIndex;
    mesh->GenMeshDesc(num_tris, indices, num_verts, num_norms, num_texs, verts, norms, texs);
    mSceneInfo.mObjs.insert(std::make_pair(gen_object_id(), mesh));
    
    //
    free(verts); verts = nullptr;
    free(norms); norms = nullptr;
    free(texs); texs = nullptr;
    free(indices); indices = nullptr;

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
            mSceneInfo.mObjs.insert(std::make_pair(gen_object_id(), ttp));
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
            mSceneInfo.mObjs.insert(std::make_pair(gen_object_id(), tt));
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
    mSceneInfo.mObjs.insert(std::make_pair(gen_object_id(), tpa));
    
    return true;
}

bool BARTParser::read_vectors(FILE *scene, const char *type, int *numVecs, BARTVec3 **vecs) {
    int num, q;
    BARTVec3 *verts = NULL;

    if (fscanf(scene, "%d", &num) != 1) {
        printf("Error: could not parse mesh (expected 'num_%s').\n", type);
        return false;
    }

    verts = (BARTVec3*)malloc(sizeof(BARTVec3)*num);
    if (verts == nullptr) {
        printf("Error: could not allocate memory for vertices of mesh.\n");
        return false;
    }
       
    for (q=0; q<num; ++q) {
        if (fscanf(scene, "%f %f %f ", &verts[q].x, &verts[q].y, &verts[q].z) != 3) {
            printf("Error: could not read %d %s of mesh.\n", num, type);
            return false;
        }
    }
    
    *vecs = verts;
    *numVecs = num;
    
    return true;
}

bool BARTParser::read_textures(FILE *scene, char *textureName, int *numTexs, BARTTexCoord **texs) {
    int q;
    int num_texs;
    BARTTexCoord *texs_;
    
    if (fscanf(scene, "%d", &num_texs) !=1 ) {
        printf("Error: could not parse mesh (expected 'num_texs').\n");
        return false;
    }
    
    texs_ = (BARTTexCoord*)malloc(sizeof(BARTTexCoord)*num_texs);
    if (texs_ == nullptr) {
        printf("Error: could not allocate memory for texturecoords of mesh.\n");
        return false;
    }
    
    fscanf(scene, "%s", textureName);
    for (q = 0; q < num_texs; ++q) {
        if (fscanf(scene, "%f %f", &texs_[q].u, &texs_[q].v) != 2) {
            printf("Error: could not read %d texturecoords of mesh.\n", num_texs);
            return false;
        }
    }
    
    *numTexs = num_texs;
    *texs = texs_;
    
    return true;
}

bool BARTParser::read_triangles(FILE *scene, int *num_tris, unsigned short **indices,
                                BARTVec3 *verts, BARTVec3 *norms, BARTTexCoord *texs) {
    int num;
    int q, w;
    int alloc_size;
    unsigned short *idx;
    int i, v[3], n[3], t[3];
       
    alloc_size = 3;
    
    if (norms) {
        alloc_size += 3;
    }
    
    if (texs) {
        alloc_size += 3;
    }
       
    if (fscanf(scene, "%d", &num) != 1) {
        printf("Error: could not parse mesh (expected 'num_triangles').\n");
        return  false;;
    }

    idx = (unsigned short *)malloc(num*alloc_size*sizeof(unsigned short));
    if (idx == nullptr) {
        printf("Error: could not allocate memory for indices of mesh.\n");
        return false;
    }
    
    i = 0;
    for (q = 0; q < num; ++q) {
        if (fscanf(scene, "%d %d %d", &v[0], &v[1], &v[2]) != 3) {
            printf("Error: could not read %d vertex indices of mesh.\n", num);
            return false;
        }

        if (norms) {
            if (fscanf(scene, "%d %d %d", &n[0], &n[1], &n[2]) != 3) {
                printf("Error: could not read %d set of normal indices of mesh.\n", num);
                return false;
            }
        }
    
        if (texs) {
            if (fscanf(scene, "%d %d %d", &t[0], &t[1], &t[2]) != 3) {
                printf("Error: could not read %d texturecoord indices of mesh.\n", num);
                return false;
            }
        }
        
        // indices appear in this order: [texture] [normals] vertices. []=optional
        for (w = 0; w < 3; ++w) {
            if (texs) {
                idx[i++] = t[w];
            }
            if (norms) {
                idx[i++] = n[w];
            }
            idx[i++] = v[w];
        }
    }
    
    *indices = idx;
    *num_tris = num;
    
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
    
    // go backwards with 2 levels of deep
    for (int i = 0; i < 2; ++i) {
        mObjID = path_goto_backward(mObjID);
    }
}

string BARTParser::path_goto_forward(const string& path, const string& fpath) const {
    string ret_path = path;
    ret_path += "_";
    ret_path += fpath;
    return ret_path;
}

string BARTParser::path_goto_backward(const string& path) const {
    size_t pos = path.find("_");
    if (pos == string::npos) {
        return path;
    }
    
    string ret_path = path.substr(0, pos);
    return ret_path;
}

string BARTParser::gen_object_id(void) {
    ++mObjIndex;
    string obj_id = mObjID + "+";
    obj_id += std::to_string(mObjIndex);
    return obj_id;
}

//
void BARTParser::cleanup() {
    mObjID.clear();
    
    mSceneInfo.mLights.clear();
    
    mSceneInfo.mMats.clear();
        
    for (auto &item : mSceneInfo.mObjs) {
        delete item.second;
    }
    mSceneInfo.mObjs.clear();
    
    mAnimFrameInfo.start_time = 0.0f;
    mAnimFrameInfo.end_time = 0.0f;
    mAnimFrameInfo.num_frames = 0;
    
    AnimationList *al = mpAnimList;
    while (al != nullptr) {
        if (al->animation.name != nullptr) {
            free(al->animation.name);
        }
        if (al->animation.translations != nullptr) {
            KB_PosTerminate(al->animation.translations);
        }
        if (al->animation.rotations != nullptr) {
        //    KB_RotTerminate(al->animation.rotations);
        }
        if (al->animation.scales != nullptr) {
        //    KB_PosTerminate(al->animation.scales);
        }
        if (al->animation.visibilities != nullptr) {
            free(al->animation.visibilities);
        }
        AnimationList *temp = al;
        al = al->next;
        free(temp);
    }

}

void BARTParser::reset_RTS_vectors() {
    mScale = {1.0f, 1.0f, 1.0f};
    mRotate = {1.0f, 1.0f, 1.0f};
    mTranslate = {1.0f, 1.0f, 1.0f};
    mRotationAngle = 0.0f;
}

}
