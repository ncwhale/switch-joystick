#ifndef FIXED_RING_BUFFER_HH
#define FIXED_RING_BUFFER_HH

// This is a very very special class with capacity = 128.
// unsigned char will auto wrap from 0~255.
// use other capacity will case error.
template<typename T, unsigned char capacity = 128>
class FixedRingBuffer {
	public:
		FixedRingBuffer(){}

		// Head point to next will write.
		T& head(){
			return buffer[write_offset & 0x7F];
		}
		// Tail point to next will read.
		T& tail(){
			return buffer[read_offset & 0x7F];
		}

		// move wrtie cursor to next, return new write place.
		T& write(){
			++write_offset;
			return head();
		}

		// read from tail, and move read cursor to next.
		T& read() {
			T& it = tail();
			++read_offset;
			return it;
		}

		bool isAvailable() {
			// If they are not equal, there are some data.
			return read_offset != write_offset;
		}

		unsigned char length() {
			return (write_offset - read_offset) & 0x7F;
		}
		unsigned char free() {
			return (read_offset - write_offset) & 0x7F;
		}

		void clear() {
			read_offset = write_offset;
		}
	// private: //for debug.
		T buffer[capacity];
		unsigned char read_offset;
		unsigned char write_offset;
};

#endif //FIXED_RING_BUFFER_HH
