/***********************************************************************************************************************
 *                                                                                                                     *
 * Copyright (C) 2019-2021 Julien Eychenne                                                                             *
 *                                                                                                                     *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public   *
 * License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any      *
 * later version.                                                                                                      *
 *                                                                                                                     *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied  *
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more       *
 * details.                                                                                                            *
 *                                                                                                                     *
 * You should have received a copy of the GNU General Public License along with this program. If not, see              *
 * <http://www.gnu.org/licenses/>.                                                                                     *
 *                                                                                                                     *
 * Created: 22/05/2020                                                                                                *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <phon/runtime/object.hpp>
#include <phon/runtime/class.hpp>
#include <phon/runtime.hpp>

namespace phonometrica {

Object::Object(Class *klass, bool collectable) :
		klass(klass), ref_count(1), gc_color(collectable ? GCColor::Black : GCColor::Green)
{

}

void Object::destroy()
{
	assert(klass->destroy);
	klass->destroy(this);
}

void Object::release()
{
	if (remove_reference())
	{
		destroy();
	}
	else if (collectable() && !is_purple())
	{
		mark_purple();
		auto obj = static_cast<Collectable*>(this);
		if (obj->runtime)
		{
			obj->runtime->add_candidate(obj);
		}
	}
}

bool Object::remove_reference() noexcept
{
	return --ref_count == 0;
}

bool Object::shared() const noexcept
{
	return ref_count > 1;
}

bool Object::unique() const noexcept
{
	return ref_count == 1;
}

int32_t Object::use_count() const noexcept
{
	return ref_count;
}

bool Object::collectable() const noexcept
{
	return gc_color != GCColor::Green;
}

bool Object::clonable() const noexcept
{
	return klass->clone != nullptr;
}

bool Object::comparable() const noexcept
{
	return klass->compare != nullptr;
}

bool Object::equatable() const noexcept
{
	return klass->equal != nullptr;
}

bool Object::hashable() const noexcept
{
	return klass->hash != nullptr;
}

bool Object::traversable() const noexcept
{
	return klass->traverse != nullptr;
}

bool Object::printable() const noexcept
{
	return klass->to_string != nullptr;
}

size_t Object::hash() const
{
	if (this->hashable()) {
		return klass->hash(this);
	}

	throw error("[Type error] Type % is not hashable", class_name());
}

String Object::to_string() const
{
	if (this->printable()) {
		return klass->to_string(this);
	}

	throw error("[Type error] Type % cannot be converted to string", class_name());
}

Object *Object::clone() const
{
	if (this->clonable())
	{
		return klass->clone(this);
	}

	throw error("[Type error] Type % is not cloneable", class_name());
}

void Object::traverse(const GCCallback &callback)
{
	if (this->traversable())
	{
		klass->traverse(reinterpret_cast<Collectable*>(this), callback);
	}
}

bool Object::equal(const Object *other) const
{
	if (this->equatable())
	{
		return klass->equal(this, other);
	}

	// Try to compare values instead. This will throw an error if comparison is not supported.
	return compare(other) == 0;
}

int Object::compare(const Object *other) const
{
	if (this->comparable())
	{
		return klass->compare(this, other);
	}

	throw error("[Type error] Type % does not support comparison", class_name());
}

String Object::class_name() const
{
	return klass->name();
}

const std::type_info *Object::type_info() const
{
	return klass->type_info();
}

bool Object::gc_candidate() const noexcept
{
	return gc_color == GCColor::White;
}


//---------------------------------------------------------------------------------------------------------------------

Atomic::Atomic(Class *klass) :
	Object(klass, false)
{

}


//---------------------------------------------------------------------------------------------------------------------

Collectable::Collectable(Class *klass, Runtime *runtime) :
	Object(klass, runtime != nullptr), runtime(runtime)
{

}

Collectable::~Collectable()
{
	if (is_candidate() && runtime)
	{
		runtime->remove_candidate(this);
	}
}

} // namespace phonometrica
