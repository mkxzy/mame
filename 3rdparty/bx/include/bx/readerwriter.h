/*
 * Copyright 2010-2017 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_READERWRITER_H_HEADER_GUARD
#define BX_READERWRITER_H_HEADER_GUARD

#include <alloca.h>
#include <stdarg.h> // va_list
#include <stdio.h>
#include <string.h>

#include "bx.h"
#include "allocator.h"
#include "error.h"
#include "uint32_t.h"

BX_ERROR_RESULT(BX_ERROR_READERWRITER_OPEN,         BX_MAKEFOURCC('R', 'W', 0, 1) );
BX_ERROR_RESULT(BX_ERROR_READERWRITER_READ,         BX_MAKEFOURCC('R', 'W', 0, 2) );
BX_ERROR_RESULT(BX_ERROR_READERWRITER_WRITE,        BX_MAKEFOURCC('R', 'W', 0, 3) );
BX_ERROR_RESULT(BX_ERROR_READERWRITER_EOF,          BX_MAKEFOURCC('R', 'W', 0, 4) );
BX_ERROR_RESULT(BX_ERROR_READERWRITER_ALREADY_OPEN, BX_MAKEFOURCC('R', 'W', 0, 5) );

namespace bx
{
	///
	struct Whence
	{
		enum Enum
		{
			Begin,
			Current,
			End,
		};
	};

	///
	struct BX_NO_VTABLE ReaderI
	{
		virtual ~ReaderI() = 0;
		virtual int32_t read(void* _data, int32_t _size, Error* _err) = 0;
	};

	///
	struct BX_NO_VTABLE WriterI
	{
		virtual ~WriterI() = 0;
		virtual int32_t write(const void* _data, int32_t _size, Error* _err) = 0;
	};

	///
	struct BX_NO_VTABLE SeekerI
	{
		virtual ~SeekerI() = 0;
		virtual int64_t seek(int64_t _offset = 0, Whence::Enum _whence = Whence::Current) = 0;
	};

	///
	struct BX_NO_VTABLE ReaderSeekerI : public ReaderI, public SeekerI
	{
	};

	///
	struct BX_NO_VTABLE WriterSeekerI : public WriterI, public SeekerI
	{
	};

	///
	struct BX_NO_VTABLE ReaderOpenI
	{
		virtual ~ReaderOpenI() = 0;
		virtual bool open(const char* _filePath, Error* _err) = 0;
	};

	///
	struct BX_NO_VTABLE WriterOpenI
	{
		virtual ~WriterOpenI() = 0;
		virtual bool open(const char* _filePath, bool _append, Error* _err) = 0;
	};

	///
	struct BX_NO_VTABLE CloserI
	{
		virtual ~CloserI() = 0;
		virtual void close() = 0;
	};

	///
	struct BX_NO_VTABLE FileReaderI : public ReaderOpenI, public CloserI, public ReaderSeekerI
	{
	};

	///
	struct BX_NO_VTABLE FileWriterI : public WriterOpenI, public CloserI, public WriterSeekerI
	{
	};

	///
	struct BX_NO_VTABLE MemoryBlockI
	{
		virtual void* more(uint32_t _size = 0) = 0;
		virtual uint32_t getSize() = 0;
	};

	///
	class StaticMemoryBlock : public MemoryBlockI
	{
	public:
		///
		StaticMemoryBlock(void* _data, uint32_t _size);

		///
		virtual ~StaticMemoryBlock();

		///
		virtual void* more(uint32_t _size = 0);

		///
		virtual uint32_t getSize() BX_OVERRIDE;

	private:
		void* m_data;
		uint32_t m_size;
	};

	///
	class MemoryBlock : public MemoryBlockI
	{
	public:
		///
		MemoryBlock(AllocatorI* _allocator);

		///
		virtual ~MemoryBlock();

		///
		virtual void* more(uint32_t _size = 0) BX_OVERRIDE;

		///
		virtual uint32_t getSize() BX_OVERRIDE;

	private:
		AllocatorI* m_allocator;
		void* m_data;
		uint32_t m_size;
	};

	///
	class SizerWriter : public WriterSeekerI
	{
	public:
		///
		SizerWriter();

		///
		virtual ~SizerWriter();

		///
		virtual int64_t seek(int64_t _offset = 0, Whence::Enum _whence = Whence::Current) BX_OVERRIDE;

		///
		virtual int32_t write(const void* /*_data*/, int32_t _size, Error* _err) BX_OVERRIDE;

	private:
		int64_t m_pos;
		int64_t m_top;
	};

	///
	class MemoryReader : public ReaderSeekerI
	{
	public:
		///
		MemoryReader(const void* _data, uint32_t _size);

		///
		virtual ~MemoryReader();

		///
		virtual int64_t seek(int64_t _offset, Whence::Enum _whence) BX_OVERRIDE;

		///
		virtual int32_t read(void* _data, int32_t _size, Error* _err) BX_OVERRIDE;

		///
		const uint8_t* getDataPtr() const;

		///
		int64_t getPos() const;

		///
		int64_t remaining() const;

	private:
		const uint8_t* m_data;
		int64_t m_pos;
		int64_t m_top;
	};

	///
	class MemoryWriter : public WriterSeekerI
	{
	public:
		///
		MemoryWriter(MemoryBlockI* _memBlock);

		///
		virtual ~MemoryWriter();

		///
		virtual int64_t seek(int64_t _offset = 0, Whence::Enum _whence = Whence::Current) BX_OVERRIDE;

		///
		virtual int32_t write(const void* _data, int32_t _size, Error* _err) BX_OVERRIDE;

	private:
		MemoryBlockI* m_memBlock;
		uint8_t* m_data;
		int64_t m_pos;
		int64_t m_top;
		int64_t m_size;
	};

	///
	class StaticMemoryBlockWriter : public MemoryWriter
	{
	public:
		///
		StaticMemoryBlockWriter(void* _data, uint32_t _size);

		///
		virtual ~StaticMemoryBlockWriter();

	private:
		StaticMemoryBlock m_smb;
	};

	/// Read data.
	int32_t read(ReaderI* _reader, void* _data, int32_t _size, Error* _err = NULL);

	/// Read value.
	template<typename Ty>
	int32_t read(ReaderI* _reader, Ty& _value, Error* _err = NULL);

	/// Read value and converts it to host endianess. _fromLittleEndian specifies
	/// underlying stream endianess.
	template<typename Ty>
	int32_t readHE(ReaderI* _reader, Ty& _value, bool _fromLittleEndian, Error* _err = NULL);

	/// Write data.
	int32_t write(WriterI* _writer, const void* _data, int32_t _size, Error* _err = NULL);

	/// Write repeat the same value.
	int32_t writeRep(WriterI* _writer, uint8_t _byte, int32_t _size, Error* _err = NULL);

	/// Write value.
	template<typename Ty>
	int32_t write(WriterI* _writer, const Ty& _value, Error* _err = NULL);

	/// Write value as little endian.
	template<typename Ty>
	int32_t writeLE(WriterI* _writer, const Ty& _value, Error* _err = NULL);

	/// Write value as big endian.
	template<typename Ty>
	int32_t writeBE(WriterI* _writer, const Ty& _value, Error* _err = NULL);

	/// Write formated string.
	int32_t writePrintf(WriterI* _writer, const char* _format, ...);

	/// Skip _offset bytes forward.
	int64_t skip(SeekerI* _seeker, int64_t _offset);

	/// Seek to any position in file.
	int64_t seek(SeekerI* _seeker, int64_t _offset = 0, Whence::Enum _whence = Whence::Current);

	/// Returns size of file.
	int64_t getSize(SeekerI* _seeker);

	/// Peek data.
	int32_t peek(ReaderSeekerI* _reader, void* _data, int32_t _size, Error* _err = NULL);

	/// Peek value.
	template<typename Ty>
	int32_t peek(ReaderSeekerI* _reader, Ty& _value, Error* _err = NULL);

	/// Align reader stream.
	int32_t align(ReaderSeekerI* _reader, uint32_t _alignment, Error* _err = NULL);

	/// Align writer stream (pads stream with zeros).
	int32_t align(WriterSeekerI* _writer, uint32_t _alignment, Error* _err = NULL);

	///
	bool open(ReaderOpenI* _reader, const char* _filePath, Error* _err = NULL);

	///
	bool open(WriterOpenI* _writer, const char* _filePath, bool _append = false, Error* _err = NULL);

	///
	void close(CloserI* _reader);

} // namespace bx

#include "readerwriter.inl"

#endif // BX_READERWRITER_H_HEADER_GUARD
