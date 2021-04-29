/* date = April 23rd 2021 3:39 pm */

#ifndef WAVE_FORMAT_H
#define WAVE_FORMAT_H

////////////////////////////////
// NOTE(allen): Types defined for the "WAV" file format

// NOTE(allen): I am not clear on whether I want to name this
// "wave_format" and namespace it with "wave" or "riff_format"
// and namespace it with "riff". It doesn't matter a whole lot
// but I would certainly like it if my names reflected the
// correct level of reusability for these definitions. For now
// I went with "wave" because that implies less reusability,
// which seems like the right default assumptions.
// TODO(allen): study the specs more for clarification.

enum{
    WAVE_ID_RIFF = AsciiID4('R', 'I', 'F', 'F'),
    WAVE_ID_WAVE = AsciiID4('W', 'A', 'V', 'E'),
    WAVE_ID_fmt  = AsciiID4('f', 'm', 't', ' '),
    WAVE_ID_data = AsciiID4('d', 'a', 't', 'a'),
};

struct WAVE_RiffMasterChunkHeader{
    U32 riff_id;
    U32 size;
    U32 wave_id;
};

struct WAVE_RiffSubChunkHeader{
    U32 id;
    U32 size;
};

////////////////////////////////
// NOTE(allen): "fmt " Sub Chunk

typedef U16 WAVE_FormatTag;
enum{
    WAVE_FormatTag_PCM = 0x0001,
    WAVE_FormatTag_IEEE_FLOAT = 0x0003,
    WAVE_FormatTag_ALAW = 0x0006,
    WAVE_FormatTag_MULAW = 0x0007,
    WAVE_FormatTag_EXTENSIBLE = 0xFFFE,
};

// definitions:
//  sample  - one value in a sequence of values forming a signal
//  channel - one (of several) "parallel" signals
//            i.e. signals with the same number of samples
//  block   - a sample for each channel, at a single sequence position

struct WAVE_RiffSubChunk_fmt{
    // bytes [0, 15]
    WAVE_FormatTag format_tag;
    U16 channel_count;
    U32 block_per_second;
    U32 bytes_per_second;
    U16 bytes_per_block;
    U16 bits_per_sample;
    
    // bytes [16, 17]
    U16 extension_size;
    
    // bytes [18, 39]
    U16 valid_bits_per_sample;
    U32 channel_mask;
    U8  sub_format[16];
    
    // NOTE(allen): According to the reference (need to test)
    //  In PCM bits_per_sample may not be a multiple of 8
    //   to indicate that less than all of the bits are significant.
    //   Still stride by a multiple of 8 bits.
    //  In Non-PCM bits_per_sample must be a multiple of 8,
    //   and valid_bits_per_sample (if included) can be used to
    //   indicate the number of significant bits.
};

#endif //WAVE_FORMAT_H
