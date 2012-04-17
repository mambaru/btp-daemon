#pragma once

namespace fas { namespace json {

template<>
class serializerT< value<double> >
{
public:
	// serialize
	template<typename P>
	P operator()(double v, P beg)
	{
		char buf[32] = { '\0' };
		const char* s = buf;
		snprintf(buf, sizeof(buf), "%.4f", v);
		while(*s != '\0')
			*(beg++) = *(s++);
		return beg;
	}

	// unserialize
	template<typename P>
	P operator()(double& v, P beg, P end)
	{
		if(parser::is_null(beg, end)) {
			v = 0.0;
			return parser::parse_null(beg, end);
		}
		if(beg == end)
			throw unexpected_end_fragment();
		v = atof(beg);
		while(beg != end) {
			if((*beg < '0' || *beg > '9') && *beg != '.' && *beg != '-')
				break;
			++beg;
		}
		return beg;
	}
};

template<>
struct value<double>
{
	typedef double target;
	typedef serializerT< value<double> > serializer;
};

}}
