#include "game/input/mouse_selection.h"

double x_position = 0.f;
double y_position = 0.f;

void jactorio::game::mouse_selection::set_cursor_position(const double x_pos, const double y_pos) {
	x_position = x_pos;
	y_position = y_pos;
}

double jactorio::game::mouse_selection::get_position_x() {
	return x_position;
}

double jactorio::game::mouse_selection::get_position_y() {
	return y_position;
}
