// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 04/23/2020

#ifndef JACTORIO_GAME_INPUT_INPUT_KEY_H
#define JACTORIO_GAME_INPUT_INPUT_KEY_H
#pragma once

namespace jactorio::game
{
	enum class InputKey
	{
		none,

		// left, right
		mouse1,
		mouse2,

		k0,
		k1,
		k2,
		k3,
		k4,
		k5,
		k6,
		k7,
		k8,
		k9,
		a,
		b,
		c,
		d,
		e,
		f,
		g,
		h,
		i,
		j,
		k,
		l,
		m,
		n,
		o,
		p,
		q,
		r,
		s,
		t,
		u,
		v,
		w,
		x,
		y,
		z,
		grave,
		tab,
		space,
		escape
	};

	enum class InputAction
	{
		none,

		// First pressed down
		key_down,

		// While pressed down, before repeat
		key_pressed,
		// Press down enough to repeat
		key_repeat,

		/// pressed and repeat
		key_held,

		// Key lifted
		key_up
	};

	enum class InputMod
	{
		none,
		alt,
		control,
		shift,
		super,
		caps_lk
	};
}

#endif // JACTORIO_GAME_INPUT_INPUT_KEY_H
