/*
 * COPYRIGHT (C) 2017-2021, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef __ASIO2_MQTT_AOP_UNSUBSCRIBE_HPP__
#define __ASIO2_MQTT_AOP_UNSUBSCRIBE_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/base/iopool.hpp>

#include <asio2/base/detail/function_traits.hpp>
#include <asio2/base/detail/util.hpp>

#include <asio2/mqtt/message_util.hpp>

namespace asio2::detail
{
	template<class caller_t>
	class mqtt_aop_unsubscribe
	{
		friend caller_t;

	protected:
		// must be server
		template<class Message, class Response>
		inline bool _before_unsubscribe_callback(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, Message& msg, Response& rep)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg, rep);

			using message_type  [[maybe_unused]] = typename detail::remove_cvref_t<Message>;
			using response_type [[maybe_unused]] = typename detail::remove_cvref_t<Response>;

			if constexpr (caller_t::is_session())
			{
				rep.packet_id(msg.packet_id());

				mqtt::utf8_string_set& topic_filters = msg.topic_filters();

				topic_filters.for_each([&](/*mqtt::utf8_string*/auto& str)
				{
					auto[share_name, topic_filter] = mqtt::parse_topic_filter(str.data_view());

					if (!share_name.empty())
					{
						caller->shared_targets_.erase(caller_ptr, share_name, topic_filter);
					}

					std::size_t removed = 0;

					auto handle = caller->subs_map_.lookup(topic_filter);
					if (handle)
					{
						//handles_.erase(handle.value());
						removed = caller->subs_map_.erase(handle.value(), caller->client_id());
					}

					// The Payload contains a list of Reason Codes.
					// https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901194
					if constexpr (std::is_same_v<response_type, mqtt::v5::unsuback>)
					{
						if /**/ (topic_filter.empty())
							rep.add_reason_codes(detail::to_underlying(mqtt::error::topic_filter_invalid));
						else if (removed == 0)
							rep.add_reason_codes(detail::to_underlying(mqtt::error::no_subscription_existed));
						else
							rep.add_reason_codes(detail::to_underlying(mqtt::error::success));
					}
					else
					{
						std::ignore = removed;
					}
				});

				return true;
			}
			else
			{
				return true;
			}
		}

		// must be server
		inline void _before_user_callback_impl(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v3::unsubscribe& msg, mqtt::v3::unsuback& rep)
		{
			if (!_before_unsubscribe_callback(ec, caller_ptr, caller, msg, rep))
				return;
		}

		// must be server
		inline void _before_user_callback_impl(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v4::unsubscribe& msg, mqtt::v4::unsuback& rep)
		{
			if (!_before_unsubscribe_callback(ec, caller_ptr, caller, msg, rep))
				return;
		}

		// must be server
		inline void _before_user_callback_impl(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v5::unsubscribe& msg, mqtt::v5::unsuback& rep)
		{
			if (!_before_unsubscribe_callback(ec, caller_ptr, caller, msg, rep))
				return;
		}

		inline void _after_user_callback_impl(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v3::unsubscribe& msg, mqtt::v3::unsuback& rep)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg, rep);
		}

		inline void _after_user_callback_impl(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v4::unsubscribe& msg, mqtt::v4::unsuback& rep)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg, rep);
		}

		inline void _after_user_callback_impl(error_code& ec, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller, mqtt::v5::unsubscribe& msg, mqtt::v5::unsuback& rep)
		{
			detail::ignore_unused(ec, caller_ptr, caller, msg, rep);

		}
	};
}

#endif // !__ASIO2_MQTT_AOP_UNSUBSCRIBE_HPP__
