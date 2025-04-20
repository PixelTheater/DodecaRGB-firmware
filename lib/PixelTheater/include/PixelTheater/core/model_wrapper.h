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

    const Face& face(size_t index) const override {
        // Use clamping for bounds checking
        size_t count = faceCount(); 
        if (index >= count) { 
             if (count == 0) return dummyFaceRef();
            index = count - 1; 
        }
        // Delegate to concrete model's proxy
        return concrete_model_->faces[index];
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

    // Potential helper to access the underlying concrete model if needed 
    // elsewhere (e.g., during Theater setup), but maybe not ideal 
    // to expose publicly if strict interface separation is desired.
    // Model<TModelDef>* getConcreteModel() { return concrete_model_.get(); }
    // const Model<TModelDef>* getConcreteModel() const { return concrete_model_.get(); }

private:
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