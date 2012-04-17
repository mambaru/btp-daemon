#include <fas/mux/best_mux.hpp>
#include <fas/mux/imux.hpp>
#include <fas/inet/server.hpp>

namespace ai = ::fas::inet;
namespace am = ::fas::mux;
namespace aa = ::fas::aop;
namespace asi = ::fas::system::inet;
namespace af = ::fas::filter;

namespace common {

	struct ad_udp_write
	{
		template<class T> void operator()(T& t, const char *d, size_t s)
		{
			t._udp_write(d,s);
		}
	};
	struct ad_udp_write_advice : aa::advice<
		aa::tag< ::fas::filter::_writer_>,
		ad_udp_write
	> {};


	template< template <typename,typename> class C, typename A>
	struct server_udp : public C<
		typename aa::aspect_merge<
			A, aa::aspect<common::ad_udp_write_advice>
		>::type, ai::connection<aa::aspect<>, am::mux_filter_base<> >
	>
	{
		typedef server_udp<C,A> self;
		typedef int desc_type;
		typedef am::imux<desc_type> imux;

		desc_type _udp;
		char _buffer[8192];
		fas::system::inet::address_t _local_address;////typedef std::vector<unsigned char> address_t;
		struct sockaddr_in _remote_address;
		imux* _mux;
		bool _nonblock;

		void set_mux(imux *mux) { _mux = mux; }
		void set_nonblock(bool value = true)  { _nonblock = value; }
		void start(unsigned short port) { start("0.0.0.0", port); }
		template<typename T>
		void start_on_same_socket(const T& t) {
			_udp = t._udp;
			if (_mux) _mux->set_rhandler( _udp, this);
		}
		void start(const std::string& addr, unsigned short port)
		{
			int optval, rc; socklen_t optlen = sizeof(optval);
			_udp = asi::socket(asi::IPv4, asi::UDP );
			optval =  16777216;
			rc = ::setsockopt(_udp, SOL_SOCKET, SO_RCVBUF, &optval, optlen);
			if (rc < 0)
			{
				int err = errno;
				fprintf(stderr, "setsockopt failed to set SO_RCVBUF in %d, error = %s\n ", optval, strerror(err));
				fprintf(stderr, "some data gramms will be lost\n");
			}
			_local_address = asi::create_address(addr, port, asi::IPv4 );
			asi::bind(_udp, _local_address);
			if (_mux) _mux->set_rhandler( _udp, this);
		}
		virtual void ready_read( desc_type d)
		{
			socklen_t fromlen = sizeof(_remote_address);
			size_t s = ::recvfrom(_udp, _buffer, sizeof(_buffer), 0, (sockaddr*)&_remote_address, &fromlen );
			this->get_aspect().template get<af::_on_read_>()( *this, _buffer, s);//call ad_set ad_get
		}
		void _udp_write( const char* d, size_t s )
		{
			sendto(_udp, d, s, 0, (const struct sockaddr *)&_remote_address, sizeof(_remote_address) );
		}
		virtual void ready_write( desc_type d) {}
		virtual void ready_urgent(desc_type d) {}
		virtual void ready_error(desc_type d) {}
		virtual bool release(bool lock = false) {return false;}
	};

};
