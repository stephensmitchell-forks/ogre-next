/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/
#ifndef _OgreHlmsDatablock_H_
#define _OgreHlmsDatablock_H_

#include "OgreStringVector.h"
#include "OgreHlmsCommon.h"
#include "OgreHeaderPrefix.h"

namespace Ogre
{
    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Resources
    *  @{
    */

    /** A macro block contains settings that will rarely change, and thus are common to many materials.
        This is very analogous to D3D11_RASTERIZER_DESC. @See HlmsDatablock
        Up to 32 different blocks are allowed!
    */
    struct HlmsMacroblock
    {
        uint8               mId;
        bool                mDepthCheck;
        bool                mDepthWrite;
        CompareFunction     mDepthFunc;
        float               mDepthBiasConstant;
        float               mDepthBiasSlopeScale;

        bool                mAlphaToCoverageEnabled;
        CullingMode         mCullMode;
        PolygonMode         mPolygonMode;
        void                *mRsData;       ///Render-System specific data

        HlmsMacroblock();

        bool operator != ( const HlmsMacroblock &_r ) const
        {
            //Don't include the ID in the comparision
            return memcmp( &mDepthCheck, &_r.mDepthCheck,
                           ((char*)&mPolygonMode - (char*)&mDepthCheck) + sizeof( PolygonMode ) ) != 0;
        }
    };

    /** A blend block contains settings that rarely change, and thus are common to many materials.
        The reasons this structure isn't joined with @HlmsMacroblock is that:
            * The D3D11 API makes this distinction (much higher API overhead if we
              change i.e. depth settings) due to D3D11_RASTERIZER_DESC.
            * This block contains information of whether the material is transparent.
              Transparent materials are sorted differently than opaque ones.
        Up to 32 different blocks are allowed!
    */
    struct HlmsBlendblock
    {
        uint8               mId;
        /// Used to determine if separate alpha blending should be used for color and alpha channels
        bool                mSeparateBlend;

        SceneBlendFactor    mSourceBlendFactor;
        SceneBlendFactor    mDestBlendFactor;
        SceneBlendFactor    mSourceBlendFactorAlpha;
        SceneBlendFactor    mDestBlendFactorAlpha;

        // Blending operations
        SceneBlendOperation mBlendOperation;
        SceneBlendOperation mBlendOperationAlpha;

        void                *mRsData;       ///Render-System specific data

        HlmsBlendblock();
        bool operator != ( const HlmsBlendblock &_r ) const
        {
            //Don't include the ID in the comparision
            return memcmp( &mSeparateBlend, &_r.mSeparateBlend,
                           ((char*)&mBlendOperationAlpha - (char*)&mSeparateBlend) +
                                                    sizeof( SceneBlendOperation ) ) != 0;
        }
    };

    /** An hlms datablock contains individual information about a specific material. It consists of:
            * A const pointer to an @HlmsMacroblock we do not own and may be shared by other datablocks.
            * A const pointer to an @HlmsBlendblock we do not own and may be shared by other datablocks.
            * The original properties from which this datablock was constructed.
            * This type may be derived to contain additional information.
        Derived types can cache information present in mOriginalProperties as strings, like diffuse
        colour values, etc.

        A datablock is the internal representation of the surface parameters (depth settings,
        textures to be used, diffuse colour, specular colour, etc).
        The notion of a datablock is the closest you'll get to a "material"
    */
    class _OgreExport HlmsDatablock : public PassAlloc
    {
        //Non-hot variables first (can't put them last as HlmsDatablock may be derived and
        //it's better if mShadowConstantBias is together with the derived type's variables
        HlmsParamVec mOriginalParams;
        Hlms    *mCreator;
        IdString mName;

    public:
        uint32  mTextureHash;       //TextureHash comes before macroblock for alignment reasons
        uint16  mMacroblockHash;    //Not all bits are used
        uint8   mType;      /// @See HlmsTypes
        bool    mIsOpaque;  /// Cached based on mBlendblock data
        HlmsMacroblock const *mMacroblock;
        HlmsBlendblock const *mBlendblock;  ///Don't set this directly, use @setBlendblock

        float   mShadowConstantBias;

    public:
        HlmsDatablock( IdString name, Hlms *creator,
                       const HlmsMacroblock *macroblock,
                       const HlmsBlendblock *blendblock,
                       const HlmsParamVec &params );
        virtual ~HlmsDatablock() {}
        virtual void calculateHash() {}

        IdString getName(void) const                { return mName; }
        Hlms* getCreator(void) const                { return mCreator; }
        const HlmsParamVec& getOriginalParams(void) const   { return mOriginalParams; }

        /// Call this function to set mBlendblock & mIsOpaque automatically based on input
        void setBlendblock( HlmsBlendblock const *blendblock );
    };

    /** Contains information needed by PBS (Physically Based Shading) for OpenGL ES 2.0
    */
    class _OgreExport HlmsPbsEs2Datablock : public HlmsDatablock
    {
    public:
        uint8   mFresnelTypeSizeBytes;              //4 if mFresnel is float, 12 if it is vec3
        float   mRoughness;
        float   mkDr, mkDg, mkDb;                   //kD
        float   mkSr, mkSg, mkSb;                   //kS
        float   mFresnelR, mFresnelG, mFresnelB;    //F0

        TexturePtr  mDiffuseTex;
        TexturePtr  mNormalmapTex;
        TexturePtr  mSpecularTex;
        TexturePtr  mReflectionTex;
        /*TexturePtr  mDetailMask;
        TexturePtr  mDetailMap[4];*/

        HlmsPbsEs2Datablock( IdString name, Hlms *creator,
                             const HlmsMacroblock *macroblock,
                             const HlmsBlendblock *blendblock,
                             const HlmsParamVec &params );

        virtual void calculateHash();
    };

    /** @} */
    /** @} */

}

#include "OgreHeaderSuffix.h"

#endif