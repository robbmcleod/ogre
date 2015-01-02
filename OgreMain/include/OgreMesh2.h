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
#ifndef _OgreMesh2_H_
#define _OgreMesh2_H_

#include "OgrePrerequisites.h"

#include "OgreResource.h"
#include "OgreAxisAlignedBox.h"
#include "Vao/OgreBufferPacked.h"
#include "OgreHeaderPrefix.h"


namespace Ogre {

    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Resources
    *  @{
    */

    class LodStrategy;

    /** Resource holding data about 3D mesh.
    @remarks
        This class holds the data used to represent a discrete
        3-dimensional object. Mesh data usually contains more
        than just vertices and triangle information; it also
        includes references to materials (and the faces which use them),
        level-of-detail reduction information, convex hull definition,
        skeleton/bones information, keyframe animation etc.
        However, it is important to note the emphasis on the word
        'discrete' here. This class does not cover the large-scale
        sprawling geometry found in level / landscape data.
    @par
        Multiple world objects can (indeed should) be created from a
        single mesh object - see the Entity class for more info.
        The mesh object will have it's own default
        material properties, but potentially each world instance may
        wish to customise the materials from the original. When the object
        is instantiated into a scene node, the mesh material properties
        will be taken by default but may be changed. These properties
        are actually held at the SubMesh level since a single mesh may
        have parts with different materials.
    @par
        As described above, because the mesh may have sections of differing
        material properties, a mesh is inherently a compound construct,
        consisting of one or more SubMesh objects.
        However, it strongly 'owns' it's SubMeshes such that they
        are loaded / unloaded at the same time. This is contrary to
        the approach taken to hierarchically related (but loosely owned)
        scene nodes, where data is loaded / unloaded separately. Note
        also that mesh sub-sections (when used in an instantiated object)
        share the same scene node as the parent.
    */
    class _OgreExport Mesh: public Resource
    {
        friend class SubMesh;
        friend class MeshSerializerImpl;

    public:
        typedef FastArray<Real> LodValueArray;
        typedef vector<SubMesh*>::type SubMeshVec;

    protected:
        /** A list of submeshes which make up this mesh.
            Each mesh is made up of 1 or more submeshes, which
            are each based on a single material and can have their
            own vertex data (they may not - they can share vertex data
            from the Mesh, depending on preference).
        */
        SubMeshVec mSubMeshes;

        DataStreamPtr mFreshFromDisk;

        /// Local bounding box volume.
        Aabb    mAabb;
        /// Local bounding sphere radius (centered on object).
        Real    mBoundRadius;

        /// Optional linked skeleton.
        String          mSkeletonName;
        SkeletonDefPtr  mSkeleton;

        String          mLodStrategyName;
        uint16          mNumLods;
        LodValueArray   mLodValues;

        VaoManager      *mVaoManager;

        BufferType mVertexBufferDefaultType;
        BufferType mIndexBufferDefaultType;
        bool mVertexBufferShadowBuffer;
        bool mIndexBufferShadowBuffer;

        /** A hashmap used to store optional SubMesh names.
            Translates a name into SubMesh index.
        */
        typedef HashMap<String, ushort> SubMeshNameMap;
        SubMeshNameMap mSubMeshNameMap;

        /** Loads the mesh from disk.  This call only performs IO, it
            does not parse the bytestream or check for any errors therein.
            It also does not set up submeshes, etc.  You have to call load()
            to do that.
         */
        void prepareImpl(void);
        /** Destroys data cached by prepareImpl.
         */
        void unprepareImpl(void);
        /// @copydoc Resource::loadImpl
        void loadImpl(void);
        /// @copydoc Resource::postLoadImpl
        void postLoadImpl(void);
        /// @copydoc Resource::unloadImpl
        void unloadImpl(void);
        /// @copydoc Resource::calculateSize
        size_t calculateSize(void) const;

    public:
        /** Default constructor - used by MeshManager
        @warning
            Do not call this method directly.
        */
        Mesh( ResourceManager* creator, const String& name, ResourceHandle handle,
              const String& group, VaoManager *vaoManager,
              bool isManual = false, ManualResourceLoader* loader = 0 );
        ~Mesh();

        // NB All methods below are non-virtual since they will be
        // called in the rendering loop - speed is of the essence.

        /** Creates a new SubMesh.
        @remarks
            Method for manually creating geometry for the mesh.
            Note - use with extreme caution - you must be sure that
            you have set up the geometry properly.
        */
        SubMesh* createSubMesh(void);

        /** Gets the number of sub meshes which comprise this mesh.
        */
        unsigned short getNumSubMeshes(void) const;

        /** Gets a pointer to the submesh indicated by the index.
        */
        SubMesh* getSubMesh(unsigned short index) const;
        
        /** Destroy a SubMesh with the given index. 
        @note
            This will invalidate the contents of any existing Entity, or
            any other object that is referring to the SubMesh list. Entity will
            detect this and reinitialise, but it is still a disruptive action.
        */
        void destroySubMesh(unsigned short index);

        /// Gets an iterator over the available submeshes
        const SubMeshVec& getSubMeshes(void) const      { return mSubMeshes; }

        /// Gives a name to a SubMesh
        void nameSubMesh(const String& name, ushort index);

        /// Removes a name from a SubMesh
        void unnameSubMesh(const String& name);

        /** Makes a copy of this mesh object and gives it a new name.
        @remarks
            This is useful if you want to tweak an existing mesh without affecting the original one. The
            newly cloned mesh is registered with the MeshManager under the new name.
        @param newName
            The name to give the clone.
        @param newGroup
            Optional name of the new group to assign the clone to;
            if you leave this blank, the clone will be assigned to the same
            group as this Mesh.
        */
        //MeshPtr clone(const String& newName, const String& newGroup = BLANKSTRING);

        /** Get the axis-aligned bounding box for this mesh.
        */
        const Aabb& getAabb(void) const;

        /** Gets the radius of the bounding sphere surrounding this mesh. */
        Real getBoundingSphereRadius(void) const;

        /** Manually set the bounding box for this Mesh.
        @remarks
            Calling this method is required when building manual meshes now, because OGRE can no longer 
            update the bounds for you, because it cannot necessarily read vertex data back from 
            the vertex buffers which this mesh uses (they very well might be write-only, and even
            if they are not, reading data from a hardware buffer is a bottleneck).
            @param pad If true, a certain padding will be added to the bounding box to separate it from the mesh
        */
        void _setBounds(const Aabb& bounds, bool pad = true);

        /** Manually set the bounding radius. 
        @remarks
            Calling this method is required when building manual meshes now, because OGRE can no longer 
            update the bounds for you, because it cannot necessarily read vertex data back from 
            the vertex buffers which this mesh uses (they very well might be write-only, and even
            if they are not, reading data from a hardware buffer is a bottleneck).
        */
        void _setBoundingSphereRadius(Real radius);

        /** Automatically update the bounding radius and bounding box for this Mesh.
        @remarks
        Calling this method is required when building manual meshes. However it is recommended to
        use _setBounds and _setBoundingSphereRadius instead, because the vertex buffer may not have
        a shadow copy in the memory. Reading back the buffer from video memory is very slow!
        @param pad If true, a certain padding will be added to the bounding box to separate it from the mesh
        */
        void _updateBoundsFromVertexBuffers(bool pad = false);

        /** Calculates 
        @remarks
        Calling this method is required when building manual meshes. However it is recommended to
        use _setBounds and _setBoundingSphereRadius instead, because the vertex buffer may not have
        a shadow copy in the memory. Reading back the buffer from video memory is very slow!
        */
        //void _calcBoundsFromVertexBuffer(VertexData* vertexData, AxisAlignedBox& outAABB, Real& outRadius, bool updateOnly = false);
        /** Sets the name of the skeleton this Mesh uses for animation.
        @remarks
            Meshes can optionally be assigned a skeleton which can be used to animate
            the mesh through bone assignments. The default is for the Mesh to use no
            skeleton. Calling this method with a valid skeleton filename will cause the
            skeleton to be loaded if it is not already (a single skeleton can be shared
            by many Mesh objects).
        @param skelName
            The name of the .skeleton file to use, or an empty string to use
            no skeleton
        */
        void setSkeletonName(const String& skelName);

        /** Returns true if this Mesh has a linked Skeleton. */
        bool hasSkeleton(void) const                                { return !mSkeletonName.empty(); }
        
        /** Gets a pointer to any linked Skeleton. 
        @return
            Weak reference to the skeleton - copy this if you want to hold a strong pointer.
        */
        const SkeletonDefPtr& getSkeleton(void) const               { return mSkeleton; }

        /** Gets the name of any linked Skeleton */
        const String& getSkeletonName(void) const;

        /** Assigns a vertex to a bone with a given weight, for skeletal animation. 
        @remarks    
            This method is only valid after calling setSkeletonName.
            Since this is a one-off process there exists only 'addBoneAssignment' and
            'clearBoneAssignments' methods, no 'editBoneAssignment'. You should not need
            to modify bone assignments during rendering (only the positions of bones) and OGRE
            reserves the right to do some internal data reformatting of this information, depending
            on render system requirements.
        @par
            This method is for assigning weights to the shared geometry of the Mesh. To assign
            weights to the per-SubMesh geometry, see the equivalent methods on SubMesh.
        */
        void addBoneAssignment(const VertexBoneAssignment& vertBoneAssign);

        /** Removes all bone assignments for this mesh. 
        @remarks
            This method is for modifying weights to the shared geometry of the Mesh. To assign
            weights to the per-SubMesh geometry, see the equivalent methods on SubMesh.
        */
        void clearBoneAssignments(void);

        /** Internal notification, used to tell the Mesh which Skeleton to use without loading it. 
        @remarks
            This is only here for unusual situation where you want to manually set up a
            Skeleton. Best to let OGRE deal with this, don't call it yourself unless you
            really know what you're doing.
        */
        void _notifySkeleton( v1::SkeletonPtr& pSkel );


        void setLodStrategyName( const String &name )               { mLodStrategyName = name; }

        /// Returns the name of the Lod strategy the user lod values have been calibrated for
        const String& getLodStrategyName(void) const                { return mLodStrategyName; }

        /** Returns the number of levels of detail that this mesh supports. 
        @remarks
            This number includes the original model.
        */
        uint16 getNumLodLevels(void) const;

        /** Retrieves the level of detail index for the given LOD value. 
        @note
            The value passed in is the 'transformed' value. If you are dealing with
            an original source value (e.g. distance), use LodStrategy::transformUserValue
            to turn this into a lookup value.
        */
        uint16 getLodIndex(Real value) const;

        /** Internal methods for loading LOD, do not use. */
        void _setLodInfo(unsigned short numLevels);
        /** Internal methods for loading LOD, do not use. */
        //void _setSubMeshLodFaceList(unsigned short subIdx, unsigned short level, IndexData* facedata);

        /** Removes all LOD data from this Mesh. */
        void removeLodLevels(void);

        /** Sets the policy for the vertex buffers to be used when loading
            this Mesh.
        @remarks
            By default, when loading the Mesh, immutable buffers will be used where possible
            in order to improve rendering performance.
            However, such buffers cannot be manipulated on the fly by CPU code
            (although shader code can). If you wish to use the CPU to modify these buffers,
            you should call this method. Note,
            however, that it only takes effect after the Mesh has been reloaded. Note that you
            still have the option of manually repacing the buffers in this mesh with your
            own if you see fit too, in which case you don't need to call this method since it
            only affects buffers created by the mesh itself.
        @par
            You can define the approach to a Mesh by changing the default parameters to 
            MeshManager::load if you wish; this means the Mesh is loaded with those options
            the first time instead of you having to reload the mesh after changing these options.
        @param bufferType
            The buffer type flags, which by default is BT_IMMUTABLE
        @param shadowBuffer
            If set to @c true, the vertex buffers will be created with a
            system memory shadow buffer. You should set this if you want to be able to
            read from the buffer, because reading from a hardware buffer is a no-no.
        */
        void setVertexBufferPolicy( BufferType bufferType, bool shadowBuffer = false );

        /** Sets the policy for the index buffers to be used when loading this Mesh.
            @see setVertexBufferPolicy
        */
        void setIndexBufferPolicy( BufferType bufferType, bool shadowBuffer = false );

        /** Gets the usage setting for this meshes vertex buffers. */
        BufferType getVertexBufferDefaultType(void) const       { return mVertexBufferDefaultType; }
        /** Gets the usage setting for this meshes index buffers. */
        BufferType getIndexBufferDefaultType(void) const        { return mIndexBufferDefaultType; }
        /** Gets whether or not this meshes vertex buffers are shadowed. */
        bool isVertexBufferShadowed(void) const { return mVertexBufferShadowBuffer; }
        /** Gets whether or not this meshes index buffers are shadowed. */
        bool isIndexBufferShadowed(void) const { return mIndexBufferShadowBuffer; }

        const LodValueArray* _getLodValueArray(void) const                      { return &mLodValues; }

        void importV1( v1::Mesh *mesh, bool halfPos, bool halfTexCoords );
    };

    /** @} */
    /** @} */
} // namespace Ogre

#include "OgreHeaderSuffix.h"

#endif // __Mesh_H__
