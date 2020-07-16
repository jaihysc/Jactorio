// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_CORE_FLOAT_MATH_H
#define JACTORIO_INCLUDE_CORE_FLOAT_MATH_H
#pragma once

#include <cmath>

#include "jactorio.h"

namespace jactorio::core
{
	// Defines math functions for floating point numbers

	constexpr double kPi = 3.141592653589793238462643383279502884;

	inline bool FloatEq(const float a, const float b, const float epsilon) {
		return fabs(a - b) < epsilon;
	}


	// If necessary in the future, swap these to another implementation
	using TDegree = float;

	inline double Sin(const TDegree degree) {
		return sin(degree / (180 / kPi));
	}

	inline double Cos(const TDegree degree) {
		return cos(degree / (180 / kPi));
	}

	inline double Tan(const TDegree degree) {
		return tan(degree / (180 / kPi));
	}


	// Precomputed for integer degrees
	using TIntDegree = unsigned int;

	inline double SinF(const TIntDegree degree) {
		assert(0 <= degree && degree <= 359);
		constexpr double sin_values[]{
			0.0, 0.01745240643728351, 0.03489949670250097, 0.05233595624294383, 0.0697564737441253, 0.08715574274765817,
			0.10452846326765346, 0.12186934340514748, 0.13917310096006544, 0.15643446504023087, 0.17364817766693033,
			0.1908089953765448, 0.20791169081775931, 0.22495105434386498, 0.24192189559966773, 0.25881904510252074,
			0.27563735581699916, 0.2923717047227367, 0.3090169943749474, 0.3255681544571567, 0.3420201433256687,
			0.35836794954530027, 0.374606593415912, 0.39073112848927377, 0.40673664307580015, 0.42261826174069944,
			0.4383711467890774, 0.45399049973954675, 0.4694715627858908, 0.48480962024633706, 0.49999999999999994,
			0.5150380749100542, 0.5299192642332049, 0.5446390350150271, 0.5591929034707468, 0.573576436351046, 0.5877852522924731,
			0.6018150231520483, 0.6156614753256583, 0.6293203910498374, 0.6427876096865393, 0.6560590289905073,
			0.6691306063588582, 0.6819983600624985, 0.6946583704589973, 0.7071067811865475, 0.7193398003386511,
			0.7313537016191705, 0.7431448254773941, 0.754709580222772, 0.766044443118978, 0.7771459614569709, 0.7880107536067219,
			0.7986355100472928, 0.8090169943749475, 0.8191520442889918, 0.8290375725550417, 0.8386705679454239, 0.848048096156426,
			0.8571673007021122, 0.8660254037844386, 0.8746197071393957, 0.8829475928589269, 0.8910065241883678, 0.898794046299167,
			0.9063077870366499, 0.9135454576426009, 0.9205048534524404, 0.9271838545667873, 0.9335804264972017,
			0.9396926207859083, 0.9455185755993167, 0.9510565162951535, 0.9563047559630354, 0.9612616959383189,
			0.9659258262890683, 0.9702957262759965, 0.9743700647852352, 0.9781476007338056, 0.981627183447664, 0.984807753012208,
			0.9876883405951378, 0.9902680687415704, 0.992546151641322, 0.9945218953682733, 0.9961946980917455, 0.9975640502598242,
			0.9986295347545738, 0.9993908270190958, 0.9998476951563913, 1.0, 0.9998476951563913, 0.9993908270190958,
			0.9986295347545738, 0.9975640502598242, 0.9961946980917455, 0.9945218953682734, 0.9925461516413221,
			0.9902680687415704, 0.9876883405951378, 0.984807753012208, 0.981627183447664, 0.9781476007338057, 0.9743700647852352,
			0.9702957262759965, 0.9659258262890683, 0.9612616959383189, 0.9563047559630355, 0.9510565162951536,
			0.9455185755993168, 0.9396926207859084, 0.9335804264972017, 0.9271838545667874, 0.9205048534524404, 0.913545457642601,
			0.90630778703665, 0.8987940462991669, 0.8910065241883679, 0.8829475928589271, 0.8746197071393959, 0.8660254037844387,
			0.8571673007021123, 0.8480480961564261, 0.8386705679454239, 0.8290375725550417, 0.8191520442889917,
			0.8090169943749475, 0.798635510047293, 0.788010753606722, 0.777145961456971, 0.766044443118978, 0.7547095802227721,
			0.7431448254773942, 0.7313537016191706, 0.7193398003386511, 0.7071067811865476, 0.6946583704589975,
			0.6819983600624986, 0.6691306063588583, 0.6560590289905073, 0.6427876096865395, 0.6293203910498374,
			0.6156614753256584, 0.6018150231520482, 0.5877852522924732, 0.5735764363510464, 0.5591929034707469,
			0.5446390350150273, 0.5299192642332049, 0.5150380749100544, 0.49999999999999994, 0.48480962024633717,
			0.4694715627858907, 0.45399049973954686, 0.4383711467890777, 0.4226182617406995, 0.40673664307580043,
			0.39073112848927377, 0.37460659341591224, 0.3583679495453002, 0.3420201433256689, 0.3255681544571566,
			0.3090169943749475, 0.29237170472273705, 0.2756373558169992, 0.258819045102521, 0.24192189559966773,
			0.2249510543438652, 0.20791169081775931, 0.19080899537654497, 0.17364817766693028, 0.15643446504023098,
			0.13917310096006574, 0.12186934340514755, 0.10452846326765373, 0.0871557427476582, 0.06975647374412552,
			0.05233595624294381, 0.03489949670250114, 0.01745240643728344, 1.2246467991473532e-16, -0.017452406437283192,
			-0.0348994967025009, -0.052335956242943564, -0.06975647374412527, -0.08715574274765794, -0.1045284632676535,
			-0.12186934340514731, -0.13917310096006552, -0.15643446504023073, -0.17364817766693003, -0.19080899537654472,
			-0.20791169081775907, -0.22495105434386498, -0.2419218955996675, -0.2588190451025208, -0.275637355816999,
			-0.29237170472273677, -0.3090169943749473, -0.32556815445715637, -0.34202014332566866, -0.3583679495453,
			-0.374606593415912, -0.39073112848927355, -0.4067366430758002, -0.4226182617406993, -0.43837114678907746,
			-0.4539904997395467, -0.4694715627858905, -0.48480962024633695, -0.4999999999999997, -0.5150380749100542,
			-0.5299192642332048, -0.5446390350150271, -0.5591929034707467, -0.5735764363510462, -0.587785252292473,
			-0.601815023152048, -0.6156614753256582, -0.6293203910498372, -0.6427876096865393, -0.656059028990507,
			-0.6691306063588582, -0.6819983600624984, -0.6946583704589974, -0.7071067811865475, -0.7193398003386509,
			-0.7313537016191705, -0.743144825477394, -0.754709580222772, -0.7660444431189779, -0.7771459614569706,
			-0.7880107536067221, -0.7986355100472928, -0.8090169943749473, -0.8191520442889916, -0.8290375725550414,
			-0.838670567945424, -0.848048096156426, -0.8571673007021121, -0.8660254037844384, -0.874619707139396,
			-0.882947592858927, -0.8910065241883678, -0.8987940462991668, -0.9063077870366497, -0.913545457642601,
			-0.9205048534524403, -0.9271838545667873, -0.9335804264972016, -0.9396926207859084, -0.9455185755993168,
			-0.9510565162951535, -0.9563047559630353, -0.9612616959383187, -0.9659258262890683, -0.9702957262759965,
			-0.9743700647852351, -0.9781476007338056, -0.981627183447664, -0.984807753012208, -0.9876883405951377,
			-0.9902680687415703, -0.992546151641322, -0.9945218953682734, -0.9961946980917455, -0.9975640502598242,
			-0.9986295347545738, -0.9993908270190958, -0.9998476951563913, -1.0, -0.9998476951563913, -0.9993908270190958,
			-0.9986295347545738, -0.9975640502598243, -0.9961946980917455, -0.9945218953682734, -0.992546151641322,
			-0.9902680687415704, -0.9876883405951378, -0.9848077530122081, -0.9816271834476641, -0.9781476007338056,
			-0.9743700647852352, -0.9702957262759966, -0.9659258262890684, -0.9612616959383188, -0.9563047559630354,
			-0.9510565162951536, -0.945518575599317, -0.9396926207859085, -0.9335804264972017, -0.9271838545667874,
			-0.9205048534524405, -0.9135454576426011, -0.9063077870366499, -0.898794046299167, -0.8910065241883679,
			-0.8829475928589271, -0.8746197071393961, -0.8660254037844386, -0.8571673007021123, -0.8480480961564262,
			-0.8386705679454243, -0.8290375725550416, -0.8191520442889918, -0.8090169943749476, -0.798635510047293,
			-0.7880107536067223, -0.7771459614569708, -0.7660444431189781, -0.7547095802227722, -0.7431448254773946,
			-0.7313537016191703, -0.7193398003386512, -0.7071067811865477, -0.6946583704589976, -0.6819983600624989,
			-0.6691306063588581, -0.6560590289905074, -0.6427876096865396, -0.6293203910498378, -0.6156614753256582,
			-0.6018150231520483, -0.5877852522924734, -0.5735764363510465, -0.5591929034707473, -0.544639035015027,
			-0.529919264233205, -0.5150380749100545, -0.5000000000000004, -0.4848096202463369, -0.4694715627858908,
			-0.45399049973954697, -0.4383711467890778, -0.4226182617407, -0.40673664307580015, -0.3907311284892739,
			-0.37460659341591235, -0.35836794954530077, -0.3420201433256686, -0.3255681544571567, -0.3090169943749476,
			-0.29237170472273716, -0.2756373558169998, -0.2588190451025207, -0.24192189559966787, -0.22495105434386534,
			-0.20791169081775987, -0.19080899537654467, -0.1736481776669304, -0.1564344650402311, -0.13917310096006588,
			-0.12186934340514811, -0.10452846326765342, -0.08715574274765832, -0.06975647374412564, -0.05233595624294437,
			-0.034899496702500823, -0.01745240643728356
		};
		return sin_values[degree];
	}

	inline double CosF(const TIntDegree degree) {
		assert(0 <= degree && degree <= 359);
		constexpr double cos_values[]{
			1.0, 0.9998476951563913, 0.9993908270190958, 0.9986295347545738, 0.9975640502598242, 0.9961946980917455,
			0.9945218953682733, 0.992546151641322, 0.9902680687415704, 0.9876883405951378, 0.984807753012208, 0.981627183447664,
			0.9781476007338057, 0.9743700647852352, 0.9702957262759965, 0.9659258262890683, 0.9612616959383189,
			0.9563047559630355, 0.9510565162951535, 0.9455185755993168, 0.9396926207859084, 0.9335804264972017,
			0.9271838545667874, 0.9205048534524404, 0.9135454576426009, 0.9063077870366499, 0.898794046299167, 0.8910065241883679,
			0.882947592858927, 0.8746197071393957, 0.8660254037844387, 0.8571673007021123, 0.848048096156426, 0.838670567945424,
			0.8290375725550417, 0.8191520442889918, 0.8090169943749475, 0.7986355100472928, 0.7880107536067219,
			0.7771459614569709, 0.766044443118978, 0.754709580222772, 0.7431448254773942, 0.7313537016191706, 0.7193398003386512,
			0.7071067811865476, 0.6946583704589973, 0.6819983600624985, 0.6691306063588582, 0.6560590289905073,
			0.6427876096865394, 0.6293203910498375, 0.6156614753256583, 0.6018150231520484, 0.5877852522924731,
			0.5735764363510462, 0.5591929034707468, 0.5446390350150272, 0.5299192642332049, 0.5150380749100544,
			0.5000000000000001, 0.4848096202463371, 0.46947156278589086, 0.4539904997395468, 0.43837114678907746,
			0.42261826174069944, 0.4067366430758002, 0.3907311284892737, 0.3746065934159122, 0.3583679495453004,
			0.3420201433256688, 0.32556815445715676, 0.30901699437494745, 0.29237170472273677, 0.27563735581699916,
			0.25881904510252074, 0.24192189559966767, 0.22495105434386514, 0.20791169081775945, 0.19080899537654492,
			0.17364817766693041, 0.15643446504023092, 0.13917310096006547, 0.12186934340514749, 0.10452846326765346,
			0.08715574274765814, 0.06975647374412546, 0.052335956242943966, 0.03489949670250108, 0.0174524064372836,
			6.123233995736766e-17, -0.017452406437283477, -0.034899496702500955, -0.05233595624294384, -0.06975647374412533,
			-0.08715574274765801, -0.10452846326765333, -0.12186934340514737, -0.13917310096006535, -0.1564344650402308,
			-0.1736481776669303, -0.1908089953765448, -0.20791169081775934, -0.22495105434386503, -0.24192189559966756,
			-0.25881904510252063, -0.27563735581699905, -0.29237170472273666, -0.30901699437494734, -0.32556815445715664,
			-0.3420201433256687, -0.35836794954530027, -0.37460659341591207, -0.3907311284892736, -0.40673664307580004,
			-0.42261826174069933, -0.4383711467890775, -0.4539904997395467, -0.46947156278589053, -0.484809620246337,
			-0.4999999999999998, -0.5150380749100543, -0.5299192642332048, -0.5446390350150271, -0.5591929034707467,
			-0.5735764363510462, -0.587785252292473, -0.601815023152048, -0.6156614753256583, -0.6293203910498373,
			-0.6427876096865394, -0.6560590289905072, -0.6691306063588582, -0.6819983600624984, -0.6946583704589974,
			-0.7071067811865475, -0.719339800338651, -0.7313537016191705, -0.743144825477394, -0.754709580222772,
			-0.7660444431189779, -0.7771459614569709, -0.7880107536067219, -0.7986355100472929, -0.8090169943749473,
			-0.8191520442889916, -0.8290375725550416, -0.8386705679454239, -0.848048096156426, -0.8571673007021122,
			-0.8660254037844387, -0.8746197071393957, -0.882947592858927, -0.8910065241883678, -0.8987940462991668,
			-0.9063077870366499, -0.9135454576426008, -0.9205048534524404, -0.9271838545667873, -0.9335804264972017,
			-0.9396926207859083, -0.9455185755993168, -0.9510565162951535, -0.9563047559630354, -0.9612616959383189,
			-0.9659258262890682, -0.9702957262759965, -0.9743700647852351, -0.9781476007338057, -0.981627183447664,
			-0.984807753012208, -0.9876883405951377, -0.9902680687415703, -0.992546151641322, -0.9945218953682733,
			-0.9961946980917455, -0.9975640502598242, -0.9986295347545738, -0.9993908270190958, -0.9998476951563913, -1.0,
			-0.9998476951563913, -0.9993908270190958, -0.9986295347545738, -0.9975640502598242, -0.9961946980917455,
			-0.9945218953682733, -0.9925461516413221, -0.9902680687415703, -0.9876883405951378, -0.9848077530122081,
			-0.981627183447664, -0.9781476007338057, -0.9743700647852352, -0.9702957262759965, -0.9659258262890683,
			-0.9612616959383189, -0.9563047559630354, -0.9510565162951536, -0.945518575599317, -0.9396926207859084,
			-0.9335804264972019, -0.9271838545667874, -0.9205048534524404, -0.9135454576426009, -0.90630778703665,
			-0.8987940462991669, -0.8910065241883679, -0.8829475928589271, -0.8746197071393959, -0.8660254037844388,
			-0.8571673007021123, -0.8480480961564261, -0.838670567945424, -0.8290375725550418, -0.8191520442889918,
			-0.8090169943749476, -0.798635510047293, -0.788010753606722, -0.777145961456971, -0.766044443118978,
			-0.7547095802227721, -0.7431448254773942, -0.7313537016191706, -0.7193398003386511, -0.7071067811865477,
			-0.6946583704589976, -0.6819983600624986, -0.6691306063588585, -0.6560590289905073, -0.6427876096865395,
			-0.6293203910498378, -0.6156614753256581, -0.6018150231520483, -0.5877852522924732, -0.5735764363510464,
			-0.5591929034707472, -0.544639035015027, -0.529919264233205, -0.5150380749100545, -0.5000000000000004,
			-0.48480962024633684, -0.46947156278589075, -0.4539904997395469, -0.43837114678907774, -0.42261826174069994,
			-0.4067366430758001, -0.3907311284892738, -0.3746065934159123, -0.3583679495453007, -0.34202014332566855,
			-0.32556815445715664, -0.30901699437494756, -0.2923717047227371, -0.2756373558169997, -0.25881904510252063,
			-0.24192189559966779, -0.22495105434386525, -0.2079116908177598, -0.1908089953765446, -0.17364817766693033,
			-0.15643446504023104, -0.13917310096006583, -0.12186934340514805, -0.10452846326765336, -0.08715574274765825,
			-0.06975647374412558, -0.052335956242944306, -0.03489949670250076, -0.017452406437283498, -1.8369701987210297e-16,
			0.01745240643728313, 0.03489949670250039, 0.052335956242943946, 0.06975647374412522, 0.08715574274765789,
			0.10452846326765299, 0.12186934340514768, 0.13917310096006547, 0.15643446504023067, 0.17364817766692997,
			0.19080899537654425, 0.20791169081775943, 0.22495105434386492, 0.24192189559966745, 0.2588190451025203,
			0.2756373558169994, 0.2923717047227367, 0.30901699437494723, 0.3255681544571563, 0.34202014332566816,
			0.3583679495453004, 0.37460659341591196, 0.3907311284892735, 0.40673664307579976, 0.4226182617406996,
			0.4383711467890774, 0.45399049973954664, 0.4694715627858904, 0.4848096202463365, 0.5000000000000001,
			0.5150380749100542, 0.5299192642332047, 0.5446390350150266, 0.559192903470747, 0.573576436351046, 0.5877852522924729,
			0.6018150231520479, 0.6156614753256578, 0.6293203910498375, 0.6427876096865393, 0.656059028990507, 0.6691306063588578,
			0.6819983600624986, 0.6946583704589973, 0.7071067811865474, 0.7193398003386509, 0.73135370161917, 0.7431448254773942,
			0.7547095802227719, 0.7660444431189778, 0.7771459614569706, 0.788010753606722, 0.7986355100472928, 0.8090169943749473,
			0.8191520442889916, 0.8290375725550414, 0.838670567945424, 0.848048096156426, 0.8571673007021121, 0.8660254037844384,
			0.8746197071393959, 0.8829475928589269, 0.8910065241883678, 0.8987940462991668, 0.9063077870366497, 0.913545457642601,
			0.9205048534524403, 0.9271838545667873, 0.9335804264972015, 0.9396926207859084, 0.9455185755993168,
			0.9510565162951535, 0.9563047559630353, 0.9612616959383187, 0.9659258262890683, 0.9702957262759965,
			0.9743700647852351, 0.9781476007338056, 0.981627183447664, 0.984807753012208, 0.9876883405951377, 0.9902680687415703,
			0.992546151641322, 0.9945218953682733, 0.9961946980917455, 0.9975640502598242, 0.9986295347545738, 0.9993908270190958,
			0.9998476951563913
		};
		return cos_values[degree];
	}

	inline double TanF(const TIntDegree degree) {
		assert(0 <= degree && degree <= 359);
		constexpr double tan_value[]{
			0.0, 0.017455064928217585, 0.03492076949174773, 0.052407779283041196, 0.06992681194351041, 0.08748866352592401,
			0.10510423526567646, 0.1227845609029046, 0.14054083470239145, 0.15838444032453627, 0.17632698070846498,
			0.19438030913771848, 0.2125565616700221, 0.2308681911255631, 0.24932800284318068, 0.2679491924311227,
			0.2867453857588079, 0.30573068145866034, 0.3249196962329063, 0.34432761328966527, 0.36397023426620234,
			0.3838640350354158, 0.4040262258351568, 0.42447481620960476, 0.4452286853085361, 0.4663076581549986,
			0.4877325885658614, 0.5095254494944288, 0.5317094316614788, 0.554309051452769, 0.5773502691896257, 0.6008606190275604,
			0.6248693519093275, 0.6494075931975106, 0.6745085168424265, 0.7002075382097097, 0.7265425280053609,
			0.7535540501027942, 0.7812856265067174, 0.809784033195007, 0.8390996311772799, 0.8692867378162267, 0.9004040442978399,
			0.9325150861376615, 0.9656887748070739, 0.9999999999999999, 1.0355303137905696, 1.0723687100246826,
			1.1106125148291928, 1.1503684072210094, 1.19175359259421, 1.2348971565350515, 1.2799416321930785, 1.3270448216204098,
			1.3763819204711734, 1.4281480067421144, 1.4825609685127403, 1.5398649638145827, 1.6003345290410507,
			1.6642794823505174, 1.7320508075688767, 1.8040477552714236, 1.8807264653463318, 1.9626105055051504, 2.050303841579296,
			2.1445069205095586, 2.2460367739042164, 2.355852365823753, 2.4750868534162946, 2.6050890646938005, 2.7474774194546216,
			2.904210877675822, 3.0776835371752527, 3.2708526184841404, 3.4874144438409087, 3.7320508075688776, 4.0107809335358455,
			4.331475874284153, 4.704630109478451, 5.144554015970307, 5.671281819617707, 6.313751514675041, 7.115369722384207,
			8.144346427974593, 9.514364454222587, 11.430052302761348, 14.300666256711896, 19.08113668772816, 28.636253282915515,
			57.289961630759144, 1.633123935319537e+16, -57.28996163075955, -28.636253282915614, -19.081136687728208,
			-14.30066625671192, -11.430052302761364, -9.514364454222598, -8.144346427974602, -7.115369722384213,
			-6.313751514675046, -5.671281819617711, -5.144554015970311, -4.704630109478454, -4.331475874284155,
			-4.010780933535847, -3.7320508075688794, -3.48741444384091, -3.270852618484142, -3.077683537175254,
			-2.904210877675823, -2.7474774194546225, -2.6050890646938014, -2.4750868534162955, -2.355852365823754,
			-2.246036773904217, -2.1445069205095595, -2.0503038415792956, -1.962610505505151, -1.8807264653463334,
			-1.804047755271424, -1.7320508075688783, -1.6642794823505178, -1.6003345290410511, -1.5398649638145827,
			-1.4825609685127408, -1.4281480067421142, -1.3763819204711738, -1.327044821620411, -1.2799416321930788,
			-1.234897156535052, -1.19175359259421, -1.15036840722101, -1.1106125148291928, -1.0723687100246828,
			-1.0355303137905694, -1.0000000000000002, -0.9656887748070746, -0.9325150861376618, -0.9004040442978404,
			-0.8692867378162267, -0.8390996311772804, -0.809784033195007, -0.7812856265067176, -0.753554050102794,
			-0.726542528005361, -0.7002075382097103, -0.6745085168424267, -0.649407593197511, -0.6248693519093275,
			-0.6008606190275607, -0.5773502691896257, -0.5543090514527691, -0.5317094316614787, -0.5095254494944289,
			-0.4877325885658618, -0.46630765815499864, -0.4452286853085365, -0.42447481620960476, -0.40402622583515707,
			-0.3838640350354158, -0.36397023426620256, -0.34432761328966516, -0.32491969623290645, -0.30573068145866067,
			-0.286745385758808, -0.267949192431123, -0.2493280028431807, -0.23086819112556334, -0.2125565616700221,
			-0.19438030913771864, -0.1763269807084649, -0.1583844403245364, -0.14054083470239176, -0.12278456090290465,
			-0.10510423526567673, -0.08748866352592402, -0.06992681194351064, -0.052407779283041175, -0.034920769491747904,
			-0.01745506492821751, -1.2246467991473532e-16, 0.017455064928217266, 0.034920769491747654, 0.05240777928304093,
			0.06992681194351039, 0.08748866352592378, 0.10510423526567649, 0.12278456090290442, 0.1405408347023915,
			0.15838444032453616, 0.17632698070846464, 0.1943803091377184, 0.21255656167002182, 0.2308681911255631,
			0.24932800284318044, 0.2679491924311227, 0.28674538575880776, 0.30573068145866045, 0.3249196962329062,
			0.3443276132896649, 0.3639702342662023, 0.3838640350354155, 0.4040262258351568, 0.4244748162096045,
			0.44522868530853615, 0.46630765815499836, 0.4877325885658615, 0.5095254494944287, 0.5317094316614783,
			0.5543090514527688, 0.5773502691896254, 0.6008606190275604, 0.6248693519093271, 0.6494075931975106,
			0.6745085168424263, 0.7002075382097098, 0.7265425280053607, 0.7535540501027936, 0.7812856265067173,
			0.8097840331950067, 0.8390996311772799, 0.8692867378162262, 0.9004040442978399, 0.9325150861376613,
			0.9656887748070742, 0.9999999999999997, 1.0355303137905687, 1.0723687100246824, 1.110612514829192, 1.1503684072210094,
			1.1917535925942093, 1.2348971565350502, 1.2799416321930794, 1.32704482162041, 1.3763819204711731, 1.4281480067421135,
			1.4825609685127386, 1.5398649638145834, 1.6003345290410502, 1.664279482350517, 1.7320508075688754, 1.804047755271425,
			1.8807264653463323, 1.9626105055051497, 2.0503038415792942, 2.1445069205095555, 2.246036773904217, 2.3558523658237522,
			2.4750868534162938, 2.605089064693798, 2.7474774194546243, 2.904210877675823, 3.077683537175252, 3.2708526184841364,
			3.487414443840901, 3.732050807568879, 4.010780933535844, 4.33147587428415, 4.704630109478443, 5.144554015970316,
			5.67128181961771, 6.313751514675036, 7.115369722384189, 8.144346427974556, 9.514364454222596, 11.430052302761332,
			14.300666256711871, 19.081136687728037, 28.636253282915778, 57.28996163075947, 5443746451065123.0, -57.28996163076068,
			-28.63625328291608, -19.081136687728172, -14.300666256711946, -11.43005230276138, -9.51436445422263,
			-8.14434642797458, -7.115369722384209, -6.313751514675051, -5.671281819617723, -5.144554015970326, -4.704630109478452,
			-4.331475874284157, -4.01078093353585, -3.7320508075688847, -3.487414443840906, -3.270852618484141,
			-3.0776835371752553, -2.9042108776758266, -2.7474774194546274, -2.605089064693801, -2.4750868534162964,
			-2.355852365823755, -2.246036773904219, -2.1445069205095577, -2.050303841579296, -1.9626105055051515,
			-1.8807264653463338, -1.8040477552714265, -1.732050807568877, -1.6642794823505183, -1.6003345290410513,
			-1.5398649638145847, -1.4825609685127397, -1.4281480067421146, -1.3763819204711742, -1.3270448216204112,
			-1.2799416321930803, -1.2348971565350513, -1.1917535925942102, -1.1503684072210103, -1.110612514829194,
			-1.0723687100246821, -1.0355303137905696, -1.0000000000000004, -0.9656887748070748, -0.9325150861376629,
			-0.9004040442978398, -0.8692867378162269, -0.8390996311772806, -0.809784033195008, -0.7812856265067171,
			-0.7535540501027942, -0.7265425280053612, -0.7002075382097104, -0.6745085168424275, -0.6494075931975105,
			-0.6248693519093277, -0.6008606190275608, -0.5773502691896265, -0.5543090514527687, -0.5317094316614788,
			-0.5095254494944291, -0.48773258856586194, -0.46630765815499936, -0.4452286853085361, -0.4244748162096049,
			-0.4040262258351572, -0.3838640350354164, -0.3639702342662022, -0.34432761328966527, -0.32491969623290656,
			-0.30573068145866084, -0.28674538575880865, -0.26794919243112264, -0.24932800284318082, -0.23086819112556348,
			-0.21255656167002268, -0.1943803091377183, -0.176326980708465, -0.15838444032453655, -0.1405408347023919,
			-0.12278456090290524, -0.1051042352656764, -0.08748866352592415, -0.06992681194351076, -0.052407779283041744,
			-0.03492076949174758, -0.017455064928217634
		};
		return tan_value[degree];
	}
}

#endif //JACTORIO_INCLUDE_CORE_FLOAT_MATH_H
