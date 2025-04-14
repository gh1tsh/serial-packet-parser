#include "parser.h"
#include "ring_buffer.h"

#include <gtest/gtest.h>

namespace
{
bool     callback_called       = false;
uint16_t callback_payload_type = 0;
uint16_t callback_payload_size = 0;

void
TestCallback(uint16_t payload_type, const uint8_t *payload, uint16_t payload_size)
{
        callback_called       = true;
        callback_payload_type = payload_type;
        callback_payload_size = payload_size;
}
}    // namespace

class ParserTestEnv : public ::testing::Test
{
protected:
        Parser_t     parser;
        RingBuffer_t ring_buffer;
        uint8_t      sync_seq[2] = { 0xAA, 0x55 };

        void SetUp() override
        {
                callback_called       = false;
                callback_payload_type = 0;
                callback_payload_size = 0;
                memset(&parser, 0, sizeof(parser));
                memset(&ring_buffer, 0, sizeof(ring_buffer));
        }

        void TearDown() override
        {
                // Reset after each test if needed
        }
};

TEST_F(ParserTestEnv, ParserInit_ValidParameters_ReturnsOk)
{
        ParserError_t result =
            PARSER_Init(&parser, &ring_buffer, sizeof(sync_seq), sync_seq, TestCallback);

        EXPECT_EQ(result, PARSER_OK);
        EXPECT_EQ(parser.m_state, PARSER_AWAIT_SYNC);
        EXPECT_EQ(parser.m_buffer, &ring_buffer);
        EXPECT_EQ(parser.m_sync_seq_size, sizeof(sync_seq));
        EXPECT_EQ(parser.m_sync_seq, sync_seq);
        EXPECT_NE(parser.m_callback, nullptr);
}