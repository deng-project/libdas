// libdas: DENG asset handling management library
// licence: Apache, see LICENCE file
// file: DasStructures.h - DAS file format structure implementation
// author: Karl-Mihkel Ott

#define DAS_STRUCTURES_CPP
#include <DasStructures.h>

namespace Libdas {

    // **** DasProperties **** //
    DasProperties::DasProperties(const DasProperties &_props) : 
        model(_props.model), 
        author(_props.author), 
        copyright(_props.copyright), 
        moddate(_props.moddate), 
        default_scene(_props.default_scene) {}


    DasProperties::DasProperties(DasProperties &&_props) : 
        model(std::move(_props.model)), 
        author(std::move(_props.author)), 
        copyright(std::move(_props.copyright)), 
        moddate(_props.moddate), 
        default_scene(_props.default_scene) {}


    void DasProperties::operator=(const DasProperties &_props) {
        model = _props.model;
        author = _props.author;
        copyright = _props.copyright;
        moddate = _props.moddate;
        default_scene = _props.default_scene;
    }


    void DasProperties::operator=(DasProperties &&_props) {
        model = std::move(_props.model);
        author = std::move(_props.author);
        copyright = std::move(_props.copyright);
        moddate = _props.moddate;
        default_scene = _props.default_scene;
    }


    // **** DasBuffer **** //
    DasBuffer::DasBuffer(const DasBuffer &_buf) : 
        data_ptrs(_buf.data_ptrs), 
        data_len(_buf.data_len), 
        type(_buf.type) {}


    DasBuffer::DasBuffer(DasBuffer &&_buf) : 
        data_ptrs(std::move(_buf.data_ptrs)), 
        data_len(_buf.data_len), 
        type(_buf.type) {}


    void DasBuffer::operator=(const DasBuffer &_buf) {
        data_ptrs = _buf.data_ptrs;
        data_len = _buf.data_len;
        type = _buf.type;
    }


    void DasBuffer::operator=(DasBuffer &&_buf) {
        data_ptrs = std::move(_buf.data_ptrs);
        data_len = _buf.data_len;
        type = _buf.type;
    }


    // **** DasMesh **** //
    DasMesh::DasMesh(const DasMesh &_mesh) : 
        name(_mesh.name), 
        primitive_count(_mesh.primitive_count) 
    {
        primitives = new uint32_t[primitive_count];
        for(uint32_t i = 0; i < primitive_count; i++)
            primitives[i] = _mesh.primitives[i];
    }


    DasMesh::DasMesh(DasMesh &&_mesh) : 
        name(std::move(_mesh.name)), 
        primitive_count(_mesh.primitive_count), 
        primitives(_mesh.primitives) 
    {
        _mesh.primitives = nullptr;
    }


    DasMesh::~DasMesh() {
        delete [] primitives;
    }


    void DasMesh::operator=(const DasMesh &_mesh) {
        this->~DasMesh();
        new (this) DasMesh(_mesh);
    }


    void DasMesh::operator=(DasMesh &&_mesh) {
        this->~DasMesh();
        new (this) DasMesh(_mesh);
    }


    // **** DasMeshPrimitive **** //
    DasMeshPrimitive::DasMeshPrimitive(const DasMeshPrimitive &_prim) : 
        index_buffer_id(_prim.index_buffer_id), 
        index_buffer_offset(_prim.index_buffer_offset),
        indices_count(_prim.indices_count), 
        vertex_buffer_id(_prim.vertex_buffer_id), 
        vertex_buffer_offset(_prim.vertex_buffer_offset), 
        vertex_normal_buffer_id(_prim.vertex_normal_buffer_id), 
        vertex_normal_buffer_offset(_prim.vertex_normal_buffer_offset), 
        vertex_tangent_buffer_id(_prim.vertex_tangent_buffer_id),
        vertex_tangent_buffer_offset(_prim.vertex_tangent_buffer_offset), 
        texture_count(_prim.texture_count),
        color_mul_count(_prim.color_mul_count),
        joint_set_count(_prim.joint_set_count),
        morph_target_count(_prim.morph_target_count)
    {
        // copy texture data
        if(texture_count) {
            uv_buffer_ids = new uint32_t[texture_count];
            for(uint32_t i = 0; i < texture_count; i++)
                uv_buffer_ids[i] = _prim.uv_buffer_ids[i];

            uv_buffer_offsets = new uint32_t[texture_count];
            for(uint32_t i = 0; i < texture_count; i++)
                uv_buffer_offsets[i] = _prim.uv_buffer_offsets[i];

            if(_prim.texture_ids) {
                texture_ids = new uint32_t[texture_count];
                for(uint32_t i = 0; i < texture_count; i++)
                    texture_ids[i] = _prim.texture_ids[i];
            }
        }

        // copy color multipliers
        if(color_mul_count) {
            color_mul_buffer_ids = new uint32_t[color_mul_count];
            for(uint32_t i = 0; i < color_mul_count; i++)
                color_mul_buffer_ids[i] = _prim.color_mul_buffer_ids[i];

            color_mul_buffer_offsets = new uint32_t[color_mul_count];
            for(uint32_t i = 0; i < color_mul_count; i++)
                color_mul_buffer_offsets[i] = _prim.color_mul_buffer_offsets[i];
        }

        // copy joint data
        if(joint_set_count) {
            joint_index_buffer_ids = new uint32_t[joint_set_count];
            for(uint32_t i = 0; i < joint_set_count; i++)
                joint_index_buffer_ids[i] = _prim.joint_index_buffer_ids[i];

            joint_index_buffer_offsets = new uint32_t[joint_set_count];
            for(uint32_t i = 0; i < joint_set_count; i++)
                joint_index_buffer_offsets[i] = _prim.joint_index_buffer_offsets[i];

            joint_weight_buffer_ids = new uint32_t[joint_set_count];
            for(uint32_t i = 0; i < joint_set_count; i++)
                joint_weight_buffer_ids[i] = _prim.joint_weight_buffer_ids[i];

            joint_weight_buffer_offsets = new uint32_t[joint_set_count];
            for(uint32_t i = 0; i < joint_set_count; i++)
                joint_weight_buffer_offsets[i] = _prim.joint_weight_buffer_offsets[i];
        }

        // copy morph target data
        if(morph_target_count) {
            morph_targets = new uint32_t[morph_target_count];
            for(uint32_t i = 0; i < morph_target_count; i++)
                morph_targets[i] = _prim.morph_targets[i];

            morph_weights = new float[morph_target_count];
            for(uint32_t i = 0; i < morph_target_count; i++)
                morph_weights[i] = _prim.morph_weights[i];
        }
    }


    DasMeshPrimitive::DasMeshPrimitive(DasMeshPrimitive &&_prim) : 
        index_buffer_id(_prim.index_buffer_id), 
        index_buffer_offset(_prim.index_buffer_offset),
        indices_count(_prim.indices_count), 
        vertex_buffer_id(_prim.vertex_buffer_id), 
        vertex_buffer_offset(_prim.vertex_buffer_offset), 
        vertex_normal_buffer_id(_prim.vertex_normal_buffer_id), 
        vertex_normal_buffer_offset(_prim.vertex_normal_buffer_offset), 
        vertex_tangent_buffer_id(_prim.vertex_tangent_buffer_id), 
        vertex_tangent_buffer_offset(_prim.vertex_tangent_buffer_offset),
        texture_count(_prim.texture_count),
        uv_buffer_ids(_prim.uv_buffer_ids),
        uv_buffer_offsets(_prim.uv_buffer_offsets),
        texture_ids(_prim.texture_ids),
        color_mul_count(_prim.color_mul_count),
        color_mul_buffer_ids(_prim.color_mul_buffer_ids),
        color_mul_buffer_offsets(_prim.color_mul_buffer_offsets),
        joint_set_count(_prim.joint_set_count),
        joint_index_buffer_ids(_prim.joint_index_buffer_ids), 
        joint_index_buffer_offsets(_prim.joint_index_buffer_offsets), 
        joint_weight_buffer_ids(_prim.joint_weight_buffer_ids),
        joint_weight_buffer_offsets(_prim.joint_weight_buffer_offsets),
        morph_target_count(_prim.morph_target_count),
        morph_targets(_prim.morph_targets), 
        morph_weights(_prim.morph_weights)
    {
        _prim.uv_buffer_ids = nullptr;
        _prim.uv_buffer_offsets = nullptr;
        _prim.texture_ids = nullptr;

        _prim.color_mul_buffer_ids = nullptr;
        _prim.color_mul_buffer_offsets = nullptr;

        _prim.joint_index_buffer_ids = nullptr;
        _prim.joint_index_buffer_offsets = nullptr;
        _prim.joint_weight_buffer_ids = nullptr;
        _prim.joint_weight_buffer_offsets = nullptr;

        _prim.morph_targets = nullptr;
        _prim.morph_weights = nullptr;
    }


    DasMeshPrimitive::~DasMeshPrimitive() {
        delete [] uv_buffer_ids;
        delete [] uv_buffer_offsets;
        delete [] texture_ids;

        delete [] color_mul_buffer_ids;
        delete [] color_mul_buffer_offsets;

        delete [] joint_index_buffer_ids;
        delete [] joint_index_buffer_offsets;
        delete [] joint_weight_buffer_ids;
        delete [] joint_weight_buffer_offsets;

        delete [] morph_targets;
        delete [] morph_weights;
    }


    void DasMeshPrimitive::operator=(const DasMeshPrimitive &_prim) {
        this->~DasMeshPrimitive();
        new (this) DasMeshPrimitive(_prim);
    }


    void DasMeshPrimitive::operator=(DasMeshPrimitive &&_prim) {
        this->~DasMeshPrimitive();
        new (this) DasMeshPrimitive(_prim);
    }


    // **** DasMorphTarget **** //
    DasMorphTarget::DasMorphTarget(const DasMorphTarget &_morph) :
        vertex_buffer_id(_morph.vertex_buffer_id),
        vertex_buffer_offset(_morph.vertex_buffer_offset),
        vertex_normal_buffer_id(_morph.vertex_normal_buffer_id),
        vertex_normal_buffer_offset(_morph.vertex_normal_buffer_offset),
        vertex_tangent_buffer_id(_morph.vertex_tangent_buffer_id),
        vertex_tangent_buffer_offset(_morph.vertex_tangent_buffer_offset),
        texture_count(_morph.texture_count),
        color_mul_count(_morph.color_mul_count)
    {
        // copy texture data
        if(texture_count) {
            uv_buffer_ids = new uint32_t[texture_count];
            for(uint32_t i = 0; i < texture_count; i++)
                uv_buffer_ids[i] = _morph.uv_buffer_ids[i];

            uv_buffer_offsets = new uint32_t[texture_count];
            for(uint32_t i = 0; i < texture_count; i++)
                uv_buffer_offsets[i] = _morph.uv_buffer_offsets[i];
        }

        // copy color multiplier data
        if(color_mul_count) {
            color_mul_buffer_ids = new uint32_t[color_mul_count];
            for(uint32_t i = 0; i < color_mul_count; i++)
                color_mul_buffer_ids[i] = _morph.color_mul_buffer_ids[i];

            color_mul_buffer_offsets = new uint32_t[color_mul_count];
            for(uint32_t i = 0; i < color_mul_count; i++)
                color_mul_buffer_offsets[i] = _morph.color_mul_buffer_offsets[i];
        }
    }


    DasMorphTarget::DasMorphTarget(DasMorphTarget &&_morph) :
        vertex_buffer_id(_morph.vertex_buffer_id),
        vertex_buffer_offset(_morph.vertex_buffer_offset),
        vertex_normal_buffer_id(_morph.vertex_normal_buffer_id),
        vertex_normal_buffer_offset(_morph.vertex_normal_buffer_offset),
        vertex_tangent_buffer_id(_morph.vertex_tangent_buffer_id),
        vertex_tangent_buffer_offset(_morph.vertex_tangent_buffer_offset),
        texture_count(_morph.texture_count),
        uv_buffer_ids(_morph.uv_buffer_ids),
        uv_buffer_offsets(_morph.uv_buffer_offsets),
        color_mul_count(_morph.color_mul_count),
        color_mul_buffer_ids(_morph.color_mul_buffer_ids),
        color_mul_buffer_offsets(_morph.color_mul_buffer_offsets)
    {
        _morph.uv_buffer_ids = nullptr;
        _morph.uv_buffer_offsets = nullptr;

        _morph.color_mul_buffer_ids = nullptr;
        _morph.color_mul_buffer_offsets = nullptr;
    }


    DasMorphTarget::~DasMorphTarget() {
        delete [] uv_buffer_ids;
        delete [] uv_buffer_offsets;

        delete [] color_mul_buffer_ids;
        delete [] color_mul_buffer_offsets;
    }


    void DasMorphTarget::operator=(const DasMorphTarget &_morph) {
        this->~DasMorphTarget();
        new (this) DasMorphTarget(_morph);
    }

    
    void DasMorphTarget::operator=(DasMorphTarget &&_morph) {
        this->~DasMorphTarget();
        new (this) DasMorphTarget(_morph);
    }


    // **** DasNode **** //
    DasNode::DasNode(const DasNode &_node) : 
        name(_node.name), 
        children_count(_node.children_count), 
        mesh(_node.mesh), 
        skeleton(_node.skeleton), 
        transform(_node.transform) 
    {
        if(children_count) {
            children = new uint32_t[children_count];
            for(uint32_t i = 0; i < children_count; i++)
                children[i] = _node.children[i];
        }
    }


    DasNode::DasNode(DasNode &&_node) : 
        name(std::move(_node.name)), 
        children_count(_node.children_count), 
        children(_node.children), 
        mesh(_node.mesh), 
        skeleton(_node.skeleton), 
        transform(_node.transform) 
    {
        _node.children = nullptr;
    }


    DasNode::~DasNode() {
        delete [] children;
    }


    void DasNode::operator=(const DasNode &_node) {
        this->~DasNode();
        new (this) DasNode(_node);
    }


    void DasNode::operator=(DasNode &&_node) {
        this->~DasNode();
        new (this) DasNode(_node);
    }


    // **** DasScene **** //
    DasScene::DasScene(const DasScene &_scene) : 
        name(_scene.name), 
        node_count(_scene.node_count), 
        root_count(_scene.root_count) 
    {
        if(node_count) {
            nodes = new uint32_t[node_count];
            for(uint32_t i = 0; i < node_count; i++)
                nodes[i] = _scene.nodes[i];
        }

        if(root_count) {
            roots = new uint32_t[root_count];
            for(uint32_t i = 0; i < root_count; i++)
                roots[i] = _scene.roots[i];
        }
    }


    DasScene::DasScene(DasScene &&_scene) :
        name(std::move(_scene.name)),
        node_count(_scene.node_count),
        nodes(_scene.nodes),
        root_count(_scene.root_count),
        roots(_scene.roots)
    {
        _scene.nodes = nullptr;
        _scene.roots = nullptr;
    }


    DasScene::~DasScene() {
        delete [] nodes;
        delete [] roots;
    }


    void DasScene::operator=(const DasScene &_scene) {
        this->~DasScene();
        new (this) DasScene(_scene);
    }


    void DasScene::operator=(DasScene &&_scene) {
        this->~DasScene();
        new (this) DasScene(_scene);
    }


    // **** DasSkeleton **** //
    DasSkeleton::DasSkeleton(const DasSkeleton &_skel) : 
        name(_skel.name), 
        parent(_skel.parent),
        joint_count(_skel.joint_count) 
    {
        if(_skel.joint_count) {
            joints = new uint32_t[joint_count];
            for(uint32_t i = 0; i < joint_count; i++)
                joints[i] = _skel.joints[i];
        }
    }


    DasSkeleton::DasSkeleton(DasSkeleton &&_skel) : 
        name(std::move(_skel.name)), 
        parent(_skel.parent),
        joint_count(_skel.joint_count), 
        joints(_skel.joints) 
    {
        _skel.joints = nullptr;
    }


    DasSkeleton::~DasSkeleton() {
        delete [] joints;
    }


    void DasSkeleton::operator=(const DasSkeleton &_skel) {
        this->~DasSkeleton();
        new (this) DasSkeleton(_skel);
    }


    void DasSkeleton::operator=(DasSkeleton &&_skel) {
        this->~DasSkeleton();
        new (this) DasSkeleton(_skel);
    }


    // **** DasSkeletonJoint **** //
    DasSkeletonJoint::DasSkeletonJoint(const DasSkeletonJoint &_joint) : 
        inverse_bind_pos(_joint.inverse_bind_pos), 
        name(_joint.name), 
        children_count(_joint.children_count),
        scale(_joint.scale), 
        rotation(_joint.rotation), 
        translation(_joint.translation) 
    {
        if(children_count) {
            children = new uint32_t[children_count];
            for(uint32_t i = 0; i < children_count; i++)
                children[i] = _joint.children[i];
        }
    }


    DasSkeletonJoint::DasSkeletonJoint(DasSkeletonJoint &&_joint) : 
        inverse_bind_pos(_joint.inverse_bind_pos), 
        name(std::move(_joint.name)), 
        children_count(_joint.children_count),
        children(_joint.children), 
        scale(_joint.scale), 
        rotation(_joint.rotation), 
        translation(_joint.translation) 
    {
        _joint.children = nullptr;
    }


    DasSkeletonJoint::~DasSkeletonJoint() {
        delete [] children;
    }


    void DasSkeletonJoint::operator=(const DasSkeletonJoint &_joint) {
        this->~DasSkeletonJoint();
        new (this) DasSkeletonJoint(_joint);
    }


    void DasSkeletonJoint::operator=(DasSkeletonJoint &&_joint) {
        this->~DasSkeletonJoint();
        new (this) DasSkeletonJoint(_joint);
    }


    // **** DasAnimation **** //
    DasAnimation::DasAnimation(const DasAnimation &_ani) : 
        name(_ani.name), 
        channel_count(_ani.channel_count) 
    {
        // channels are present
        if(channel_count) {
            channels = new uint32_t[channel_count];
            for(uint32_t i = 0; i < channel_count; i++)
                channels[i] = _ani.channels[i];
        }
    }


    DasAnimation::DasAnimation(DasAnimation &&_ani) : 
        name(std::move(_ani.name)), 
        channel_count(_ani.channel_count), 
        channels(_ani.channels) 
    {
        _ani.channels = nullptr;
    }


    DasAnimation::~DasAnimation() {
        delete [] channels;
    }

    
    void DasAnimation::operator=(const DasAnimation &_ani) {
        this->~DasAnimation();
        new (this) DasAnimation(_ani);
    }


    void DasAnimation::operator=(DasAnimation &&_ani) {
        this->~DasAnimation();
        new (this) DasAnimation(_ani);
    }


    // **** DasAnimationChannel **** //
    DasAnimationChannel::DasAnimationChannel(const DasAnimationChannel &_ch) :
        node_id(_ch.node_id),
        joint_id(_ch.joint_id),
        target(_ch.target),
        interpolation(_ch.interpolation),
        keyframe_count(_ch.keyframe_count),
        weight_count(_ch.weight_count)
    {
        if(keyframe_count) {
            keyframes = new float[keyframe_count];
            for(uint32_t i = 0; i < keyframe_count; i++)
                keyframes[i] = _ch.keyframes[i];

            uint32_t type_stride = 0;
            switch(target) {
                case LIBDAS_ANIMATION_TARGET_WEIGHTS:
                    type_stride = static_cast<uint32_t>(sizeof(float)) * weight_count;
                    break;

                case LIBDAS_ANIMATION_TARGET_TRANSLATION:
                    type_stride = static_cast<uint32_t>(sizeof(Libdas::Vector3<float>));
                    break;

                case LIBDAS_ANIMATION_TARGET_ROTATION:
                    type_stride = static_cast<uint32_t>(sizeof(Libdas::Quaternion));
                    break;

                case LIBDAS_ANIMATION_TARGET_SCALE:
                    type_stride = static_cast<uint32_t>(sizeof(float));
                    break;
            }

            // check if tangents should be copied
            if(interpolation == LIBDAS_INTERPOLATION_VALUE_CUBICSPLINE) {
                tangents = new char[type_stride * keyframe_count];
                for(uint32_t i = 0; i < type_stride * keyframe_count; i++)
                    tangents[i] = _ch.tangents[i];
            }

            // copy target values
            target_values = new char[type_stride * keyframe_count];
            for(uint32_t i = 0; i < type_stride * keyframe_count; i++)
                target_values[i] = _ch.target_values[i];
        }
    }


    DasAnimationChannel::DasAnimationChannel(DasAnimationChannel &&_ch) :
        node_id(_ch.node_id),
        joint_id(_ch.joint_id),
        target(_ch.target),
        interpolation(_ch.interpolation),
        keyframe_count(_ch.keyframe_count),
        weight_count(_ch.weight_count),
        keyframes(_ch.keyframes),
        tangents(_ch.tangents),
        target_values(_ch.target_values)
    {
        _ch.keyframes = nullptr;
        _ch.tangents = nullptr;
        _ch.target_values = nullptr;
    }


    DasAnimationChannel::~DasAnimationChannel() {
        delete [] keyframes;
        delete [] tangents;
        delete [] target_values;
    }
    

    void DasAnimationChannel::operator=(const DasAnimationChannel &_ch) {
        this->~DasAnimationChannel();
        new (this) DasAnimationChannel(_ch);
    }
    

    void DasAnimationChannel::operator=(DasAnimationChannel &&_ch) {
        this->~DasAnimationChannel();
        new (this) DasAnimationChannel(_ch);
    }
}
