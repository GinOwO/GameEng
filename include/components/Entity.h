#pragma once

#include <core/Input.h>

#include <graphics/Mesh.h>
#include <graphics/Shader.h>
#include <graphics/Texture.h>
#include <graphics/Specular.h>

#include <components/SharedGlobals.h>
#include <components/MeshRenderer.h>
#include <components/GameObject.h>
#include <components/GameComponent.h>

#include <string>
#include <cmath>
#include <algorithm>

class Entity : public GameObject {
	btRigidBody *rigid_body = nullptr;

    public:
	const btScalar move_impulse_factor = 10.0f;
	const btScalar jump_units = 1.0f;
	const btScalar max_move_velocity = 7.5f;
	const btScalar max_jump_velocity = 5.0f;
	bool on_ground = true;
	bool player;

	Entity(const std::string &mesh_path, const std::string &diffuse_path,
	       bool player = false)
		: player(player)
	{
		this->physics_type = 20; // Physics for person entities
		Mesh mesh = Mesh::load_mesh(mesh_path,
					    Mesh::MeshPhysicsType::ENTITY);
		Material material;

		material.add_property("diffuse",
				      Texture::load_texture(diffuse_path));

		material.add_property(
			"specular", std::shared_ptr<void>(new Specular{ 0, 0 },
							  Specular::deleter));

		this->add_component(new MeshRenderer(mesh, material));

		if (SharedGlobals::get_instance().current_rigid_body) {
			this->rigid_body =
				SharedGlobals::get_instance().current_rigid_body;
			rigid_body->setUserPointer(this);

			SharedGlobals::get_instance().rigid_bodies.push_back(
				rigid_body);
		}
	}

	void update(float delta) override
	{
		if (rigid_body) {
			btVector3 velocity = rigid_body->getLinearVelocity();

			btScalar x = velocity.getX();
			btScalar y = velocity.getY();
			btScalar z = velocity.getZ();

			btScalar xy_length = std::sqrt(x * x + y * y);

			if (xy_length > max_move_velocity) {
				btScalar scale = max_move_velocity / xy_length;

				velocity.setX(x * scale);
				velocity.setY(y * scale);
				velocity.setZ(std::min(z, max_jump_velocity));

				rigid_body->setLinearVelocity(velocity);
			}
			btTransform world_transform =
				rigid_body->getWorldTransform();
			btVector3 position = world_transform.getOrigin();
			transform.set_translation(Vector3f(position.getX(),
							   position.getY(),
							   position.getZ()));
		}

		GameObject::update();
	}

	void move(const Vector3f &direction, float amount) noexcept
	{
		if (rigid_body) {
			btVector3 impulse(direction.getX() * amount,
					  direction.getY() * amount,
					  direction.getZ() * amount);
			rigid_body->applyCentralImpulse(impulse);
		}
	}

	void input(float delta) override
	{
		static Input &input_handler = Input::get_instance();
		if (player) {
			if (input_handler.is_key_pressed(GLFW_KEY_W)) {
				move(transform.get_rotation().get_forward(),
				     move_impulse_factor * delta);
			}
			if (input_handler.is_key_pressed(GLFW_KEY_A)) {
				move(transform.get_rotation().get_left(),
				     move_impulse_factor * delta);
			}
			if (input_handler.is_key_pressed(GLFW_KEY_S)) {
				move(transform.get_rotation().get_backward(),
				     move_impulse_factor * delta);
			}
			if (input_handler.is_key_pressed(GLFW_KEY_D)) {
				move(transform.get_rotation().get_right(),
				     move_impulse_factor * delta);
			}

			if (on_ground &&
			    input_handler.is_key_pressed(GLFW_KEY_SPACE)) {
				move(transform.get_rotation().get_up(),
				     jump_units);
			}
		}
		on_ground = false;
		GameObject::input(delta);
	}

	void handle_collision(GameObject *obj) override
	{
		if (obj->physics_type == 10) {
			on_ground = true;
		}
	}
};