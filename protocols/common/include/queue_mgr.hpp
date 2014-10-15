/*
 * queue_mgr.hpp
 *
 *  Created on: Mar 6, 2011
 *      Author: rjoshi
 *  Joshi Inc Reserved 2010
 */

#ifndef JOSHI_QUEUE_MGR_HPP_
#define JOSHI_QUEUE_MGR_HPP_

namespace arawat {
namespace protocol {

class thread_managenent {
public:

	static bool create();
};
class lb_criteria {
public:
	static int load_balance(unsigned pending_task, unsigned curr_threads) {
		if((pending_task/ curr_threads) > 10) {
			return 1;
		}else if((pending_task/curr_threads) < 1) {
			return -1;
		}
		return 0;
	}
};
template<typename T, typename LB=lb_criteria >
class queue_mgr {

};
}
}
#endif /* JOSHI_QUEUE_MGR_HPP_ */
