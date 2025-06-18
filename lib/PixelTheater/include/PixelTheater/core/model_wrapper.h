#pragma once

#include "PixelTheater/core/imodel.h"    // Correct path
#include "PixelTheater/model/model.h" // The concrete Model class
#include "PixelTheater/model/point.h"
#include "PixelTheater/model/face.h"
#include <cstddef> // size_t
#include <stdexcept> // std::out_of_range
#include <memory> // std::unique_ptr
#include <utility> // std::move
#include <cassert> // assert

namespace PixelTheater {

/**
 * @brief Concrete implementation of IModel wrapping a Model<TModelDef> instance.
 * 
 * @tparam TModelDef The model definition struct.
 */
template<typename TModelDef>
class ModelWrapper : public IModel {
private:
    // Store the concrete model instance. 
    // Using unique_ptr to manage lifetime, though it could also be
    // constructed in-place if the wrapper owns it directly.
    std::unique_ptr<Model<TModelDef>> concrete_model_;

public:
    /**
     * @brief Construct a wrapper by taking ownership of a concrete model instance.
     *
     * @param concrete_model A unique_ptr to the Model<TModelDef> instance.
     */
    explicit ModelWrapper(std::unique_ptr<Model<TModelDef>> concrete_model)
        : concrete_model_(std::move(concrete_model))
    {
        assert(concrete_model_ != nullptr && "ModelWrapper: concrete_model cannot be null");
    }

    // Disable copy operations as unique_ptr is not copyable
    ModelWrapper(const ModelWrapper&) = delete;
    ModelWrapper& operator=(const ModelWrapper&) = delete;

    // Allow move operations (transfers ownership of concrete_model_)
    ModelWrapper(ModelWrapper&&) noexcept = default;
    ModelWrapper& operator=(ModelWrapper&&) noexcept = default;

    ~ModelWrapper() override = default;

    // Implement IModel methods by delegating and clamping
    const Point& point(size_t index) const override {
        // Use clamping for bounds checking
        size_t count = pointCount(); // Use own method for count
        if (index >= count) { 
            if (count == 0) return dummyPointRef();
            index = count - 1; 
        }
        // Delegate to concrete model's proxy
        return concrete_model_->points[index]; 
    }

    size_t pointCount() const noexcept override {
        // Delegate to concrete model's proxy
        return concrete_model_ ? concrete_model_->points.size() : 0;
    }

    const Face& face(size_t geometric_position) const override {
        // Use clamping for bounds checking
        size_t count = faceCount(); 
        if (geometric_position >= count) { 
             if (count == 0) return dummyFaceRef();
            geometric_position = count - 1; 
        }
        // Use the model's remapping face() method, but we need the underlying Face
        // since our Model::face() returns a FaceProxy, not a Face&
        auto face_proxy = concrete_model_->face(static_cast<uint8_t>(geometric_position));
        // Get the actual Face reference from the proxy by accessing the internal Face
        // We need to find the logical face ID that the proxy represents
        uint8_t logical_face_id = face_proxy.id();
        return concrete_model_->faces[logical_face_id];
    }

    size_t faceCount() const noexcept override {
        // Delegate to concrete model's proxy
        return concrete_model_ ? concrete_model_->faces.size() : 0;
    }

    /**
     * @brief Get the pre-calculated sphere radius from the model definition.
     * @return The sphere radius.
     */
    float getSphereRadius() const override {
        // Check if concrete model exists and then access the static member
        if (!concrete_model_) { 
            // Log::warning("getSphereRadius called on uninitialized ModelWrapper"); // RTTI/Logging might not be available here
            return 0.0f; 
        } 
        return TModelDef::SPHERE_RADIUS;
    }

    /**
     * @brief Get the face ID connected to a specific edge of a face.
     * @param geometric_position The geometric position of the face.
     * @param edge_index The edge index within the face (0-based).
     * @return Connected geometric position, or -1 if no connection or invalid edge.
     */
    int8_t face_at_edge(uint8_t geometric_position, uint8_t edge_index) const override {
        if (!concrete_model_ || geometric_position >= faceCount()) {
            return -1;
        }
        
        // Use the model's face_at_edge method which handles geometric position mapping
        return concrete_model_->face_at_edge(geometric_position, edge_index);
    }

    /**
     * @brief Get the number of edges for a specific face.
     * @param geometric_position The geometric position of the face.
     * @return Number of edges for the face.
     */
    uint8_t face_edge_count(uint8_t geometric_position) const override {
        if (!concrete_model_ || geometric_position >= faceCount()) {
            return 0;
        }
        
        // Use the model's face_edge_count method which handles geometric position mapping
        return concrete_model_->face_edge_count(geometric_position);
    }

    /**
     * @brief Get LED group by name for a specific face
     * @param face_id The face ID
     * @param group_name The name of the group (e.g., "center", "edge0", "ring1")
     * @return LED group interface for iteration and access
     */
    std::unique_ptr<ILedGroup> face_group(uint8_t face_id, const char* group_name) const override {
        if (!concrete_model_) {
            return std::make_unique<EmptyLedGroup>();
        }
        return concrete_model_->face_group(face_id, group_name);
    }
    
    /**
     * @brief Get all available group names for a specific face
     * @param face_id The face ID
     * @return Vector of group names available for this face
     */
    std::vector<const char*> face_group_names(uint8_t face_id) const override {
        if (!concrete_model_) {
            return std::vector<const char*>();
        }
        return concrete_model_->face_group_names(face_id);
    }

    /**
     * @brief Perform comprehensive model validation
     * @param check_geometric_validity If true, perform expensive geometric checks
     * @param check_data_integrity If true, perform data integrity checks
     * @return Detailed validation results
     */
    ModelValidation validate_model(bool check_geometric_validity = true, 
                                 bool check_data_integrity = true) const override {
        if (!concrete_model_) {
            ModelValidation result;
            result.errors.add_error("ModelWrapper: No concrete model instance");
            result.failed_checks = 1;
            result.total_checks = 1;
            result.is_valid = false;
            return result;
        }
        return concrete_model_->validate_model(check_geometric_validity, check_data_integrity);
    }

    // Potential helper to access the underlying concrete model if needed 
    // elsewhere (e.g., during Theater setup), but maybe not ideal 
    // to expose publicly if strict interface separation is desired.
    // Model<TModelDef>* getConcreteModel() { return concrete_model_.get(); }
    // const Model<TModelDef>* getConcreteModel() const { return concrete_model_.get(); }

private:
    // Empty LED group for error cases
    class EmptyLedGroup : public ILedGroup {
    public:
        CRGB& operator[](size_t i) override {
            static CRGB dummy = CRGB::Black;
            return dummy;
        }
        
        size_t size() const override {
            return 0;
        }
    };

    // Static dummy instances for clamping/error returns
    static const Point& dummyPointRef() { 
        static const Point dummy; 
        return dummy; 
    }
    static const Face& dummyFaceRef() { 
        static const Face dummy; 
        return dummy; 
    }
};

} // namespace PixelTheater 