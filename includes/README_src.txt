BART: A Benchmark for Animated Ray Tracing
==============================================
by Jonas Lext, Ulf Assarsson, and Tomas Möller
May 2000
----------------------------------------------
The most important file here is parse.c which
is a simple parser of the Animated File Format
(AFF). This files uses the others files here:
animation.c  
animation.h  
kbsplpos.c   
kbsplpos.h   
kbsplrot.c   
kbsplrot.h
parse.h
quat.c       
quat.h
texture.c
texture.h

To animate the viewer (camera), use the
GetCamera() routine from animation.*

To apply a dynamic transform to an object,
see the following source code (for OpenGL, but should
be easy to understand):
-------------------------------------------------------
static void renderXform(struct Xform *xf)  /* use whatever struct you have here instead of Xform */
{            
   Animation *anim;
   int vis=1;
   /* first, find the actual animation list */
   anim=FindAnimation(xf->mName,gScene.mAnimations); /* gScene.mAnimations is the global place where animation list were stored (you will probably have something else here */
   glPushMatrix();
             
   if(anim)  
   {         
      vis=_GetVisibility(anim,gScene.mTime);
      if(vis) /* is the object visible? */
      {      
         /* two ways to implement the transform */
#if 0        
         double m[4][4];
         double tm[4][4]; 
         _GetMatrix(anim,gScene.mTime,m);
         M44TRANSPOSE(tm,m);
         glMultMatrixd(&tm[0][0]);
#else        
         if(anim->translations)
         {       
            double trans[3];
             
            if(gScene.mRenderKeyframes)
               renderKeyframes(anim);
             
            _GetTranslation(anim,gScene.mTime,trans);
            glTranslated(trans[0],trans[1],trans[2]);
         }   
             
         if(anim->rotations)
         {       
            double rot[4];
            _GetRotation(anim,gScene.mTime,rot);
            glRotated(rot[3]*180.0/M_PI,rot[0],rot[1],rot[2]);
         }   
             
         if(anim->scales)
         {       
            double scale[3];
            _GetScale(anim,gScene.mTime,scale);
            glScaled(scale[0],scale[1],scale[2]);
         }      
#endif       
      }      
   }
             
   /* vis must be true if the object should be rendered */
   if(vis)
   {
      render(xf);   /* render the actual object */
   }         
             
   glPopMatrix();
}            
-------------------------------------------------------

Here is also some simple code on how we implemented the
animated triangle patches (even though we do not give
you all macros, it should be easy to figure out):

-------------------------------------------------------

/* How to get the vertices and normals of an animated triangle patch:
 * viGetAnimatedTriangle()
 *  if time is less than the time of the first tripatch, then
 *  return that tripatch, and if the time is greater than the
 *  time of the last tripatch, then return that tripatch.
 *  Otherwise, find the two tripatches which has "times" closest
 *  to the time of the animated tripatch. One of these times should
 *  be less than or equal to the time, and the other should be bigger.
 *  Then linearly interpolate between these tripatches.
 */
void viGetAnimatedTriangle(float time,struct AnimatedTriangle *at,Vec3f v[3],Vec3f n[3])
{
   if(time<=at->mTimes[0])
   { 
      V3SET(v[0],at->mVerts[3*0+0]);
      V3SET(v[1],at->mVerts[3*0+1]);
      V3SET(v[2],at->mVerts[3*0+2]);
      V3SET(n[0],at->mNorms[3*0+0]);
      V3SET(n[1],at->mNorms[3*0+1]);
      V3SET(n[2],at->mNorms[3*0+2]);
   }
   else if(time>=at->mTimes[at->mNumKeys-1])
   {         
      int idx=at->mNumKeys-1;
      V3SET(v[0],at->mVerts[3*idx+0]);
      V3SET(v[1],at->mVerts[3*idx+1]);
      V3SET(v[2],at->mVerts[3*idx+2]);
      V3SET(n[0],at->mNorms[3*idx+0]);
      V3SET(n[1],at->mNorms[3*idx+1]);
      V3SET(n[2],at->mNorms[3*idx+2]); 
   }
   else
   {
      int idx;
      float param;
      for(idx=0;idx<at->mNumKeys-1;idx++)
      {
         if(time>=at->mTimes[idx] && time<at->mTimes[idx+1])
            break;
      }
      /* linearly interpolate between idx and idx+1 */
      param=(time-at->mTimes[idx])/(at->mTimes[idx+1]-at->mTimes[idx]);
      V3LERP(v[0],param,at->mVerts[3*idx+0],at->mVerts[3*(idx+1)+0]);
      V3LERP(v[1],param,at->mVerts[3*idx+1],at->mVerts[3*(idx+1)+1]);
      V3LERP(v[2],param,at->mVerts[3*idx+2],at->mVerts[3*(idx+1)+2]);
      V3LERP(n[0],param,at->mNorms[3*idx+0],at->mNorms[3*(idx+1)+0]);
      V3LERP(n[1],param,at->mNorms[3*idx+1],at->mNorms[3*(idx+1)+1]);
      V3LERP(n[2],param,at->mNorms[3*idx+2],at->mNorms[3*(idx+1)+2]);
   }
   V3NORMALIZEF(n[0]);
   V3NORMALIZEF(n[1]);
   V3NORMALIZEF(n[2]);
}
-------------------------------------------------------
