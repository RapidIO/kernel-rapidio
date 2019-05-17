/*
 ****************************************************************************
 Copyright (c) 2015, Integrated Device Technology Inc.
 Copyright (c) 2015, RapidIO Trade Association
 All rights reserved.

 Redistribution and use in source and binary forms, with or without modification,
 are permitted provided that the following conditions are met:

 1. Redistributions of source code must retain the above copyright notice, this
 list of conditions and the following disclaimer.

 2. Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.

 3. Neither the name of the copyright holder nor the names of its contributors
 may be used to endorse or promote products derived from this software without
 specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *************************************************************************
 */

#ifndef __CIRC_BUF_H__
#define __CIRC_BUF_H__

#include <iostream>
#include <string>
#include <array>

template<typename T, size_t N>
class circ_buf {
public:
	circ_buf() :
			head(0), tail(0), n(0)
	{
	}

	void push_back(T x)
	{
		/* When list is empty, don't increment tail */
		if (n == 0) {
			buffer[tail] = x;
			n++;
		} else {
			/* List is full, drop element at head */
			if (n == buffer.size()) {
				inc_head();
				n--;
			}

			/* Increment tail, then add element */
			inc_tail();
			buffer[tail] = x;
			n++;
		}
	} /* push_back() */

	void dump()
	{
		/* If empty list do nothing */
		if (n == 0) {
			return;
		}

		/* If list has 1 element, special case since head==tail */
		if (n == 1) {
			std::cout << buffer[head];
			return;
		}

		/* Save the head */
		unsigned orig_head = head;

		/* Start at head and print everything up to the tail */
		while (head != tail) {
			std::cout << buffer[head];
			if ('\n'
					!= buffer[head].c_str()[strlen(
							buffer[head].c_str())
							- 1]) {
				std::cout << std::endl;
			}
			inc_head();
		}

		/* The tail needs to be printed as well */
		std::cout << buffer[tail];

		/* Restore the head */
		head = orig_head;
	} /* dump() */

	void clear()
	{
		head = tail = n = 0;
	} /* clear() */

private:
	/* Increment head, wrapping around if needed */
	void inc_head()
	{
		if (++head == buffer.size()) {
			head = 0;
		}
	}

	/* Increment tail, wrapping around if needed */
	void inc_tail()
	{
		if (++tail == buffer.size()) {
			tail = 0;
		}
	}

	/* Buffer, head, and tail indexes */
	std::array<T, N> buffer;
	unsigned head;
	unsigned tail;
	unsigned n;
};

#endif /* __CIRC_BUF_H__ */
