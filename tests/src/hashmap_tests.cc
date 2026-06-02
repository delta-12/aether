#include <cstddef>
#include <cstdint>
#include <span>

#include <gmock/gmock.h>
#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

#include "err.h"
#include "hashmap.h"

#define SPAN_FROM_VALUE(value, size) std::span<std::uint8_t>(static_cast<std::uint8_t *>(value), size)

class Callbacks
{
public:
    virtual void Callback(const void *const key, const std::size_t key_size, void *const value, const std::size_t value_size, const void *const arg) const = 0;
};

class MockCallbacks : public Callbacks
{
public:
    MOCK_METHOD(void, Callback, (const void *const key, const std::size_t key_size, void *const value, const std::size_t value_size, const void *const arg), (const, override));
};

class Hashmap : public testing::Test
{
protected:
    static void Callback(const void *const key,
                         const std::size_t key_size,
                         void *const value,
                         const std::size_t value_size,
                         const void *const arg)
    {
        mock_callbacks_->Callback(key, key_size, value, value_size, arg);
    }

    void SetUp() override
    {
        mock_callbacks_ = new MockCallbacks;
    }

    void TearDown() override
    {
        a_Hashmap_Deinitialize(&hashmap);
        delete mock_callbacks_;
    }

    static MockCallbacks *mock_callbacks_;
    a_Hashmap_t hashmap;
};

MockCallbacks *Hashmap::mock_callbacks_ = nullptr;

TEST_F(Hashmap, Initialize)
{
    ASSERT_EQ(A_ERR_NULL, a_Hashmap_Initialize(nullptr));
    ASSERT_EQ(A_ERR_NONE, a_Hashmap_Initialize(&hashmap));
}

TEST_F(Hashmap, Insert)
{
    std::uint32_t key = 0x12345678U;
    std::uint8_t value[10U] = {0x00U, 0x01U, 0x02U, 0x03U, 0x04U, 0x05U, 0x06U, 0x07U, 0x08U, 0x09U};
    a_Hashmap_Initialize(&hashmap);

    ASSERT_EQ(A_ERR_NULL, a_Hashmap_Insert(nullptr, &key, sizeof(key), value, sizeof(value)));
    ASSERT_EQ(A_ERR_NULL, a_Hashmap_Insert(&hashmap, nullptr, sizeof(key), value, sizeof(value)));
    ASSERT_EQ(A_ERR_NULL, a_Hashmap_Insert(&hashmap, &key, sizeof(key), nullptr, sizeof(value)));

    ASSERT_EQ(A_ERR_SIZE, a_Hashmap_Insert(&hashmap, &key, 0U, value, sizeof(value)));
    ASSERT_EQ(A_ERR_SIZE, a_Hashmap_Insert(&hashmap, &key, sizeof(key), value, 0U));

    ASSERT_EQ(A_ERR_NONE, a_Hashmap_Insert(&hashmap, &key, sizeof(key), value, sizeof(value)));
    ASSERT_EQ(A_ERR_NONE, a_Hashmap_Insert(&hashmap, &key, sizeof(key), value, sizeof(value)));

    value[0U] = 0x10U;
    ASSERT_EQ(A_ERR_NONE, a_Hashmap_Insert(&hashmap, &key, sizeof(key), value, sizeof(value)));

    ASSERT_EQ(A_ERR_NONE, a_Hashmap_Insert(&hashmap, &key, sizeof(key), value, sizeof(value) - 1U));

    /* TODO test handling collisions and if table is full */
}

TEST_F(Hashmap, Get)
{
    std::uint32_t key_0 = 0x12345678U;
    std::uint8_t value_0[10U] = {0x00U, 0x01U, 0x02U, 0x03U, 0x04U, 0x05U, 0x06U, 0x07U, 0x08U, 0x09U};
    std::uint32_t key_1 = 0x91011121U;
    std::uint8_t value_1[10U] = {0x10U, 0x11U, 0x12U, 0x13U, 0x14U, 0x15U, 0x16U, 0x17U, 0x18U, 0x19U};
    a_Hashmap_Initialize(&hashmap);
    a_Hashmap_Insert(&hashmap, &key_0, sizeof(key_0), value_0, sizeof(value_0));
    a_Hashmap_Insert(&hashmap, &key_1, sizeof(key_1), value_1, sizeof(value_1));

    ASSERT_EQ(nullptr, a_Hashmap_Get(nullptr, &key_0, sizeof(key_0)));
    ASSERT_EQ(nullptr, a_Hashmap_Get(&hashmap, nullptr, sizeof(key_0)));
    ASSERT_EQ(nullptr, a_Hashmap_Get(&hashmap, &key_0, 0U));

    ASSERT_THAT(SPAN_FROM_VALUE(a_Hashmap_Get(&hashmap, &key_0, sizeof(key_0)), sizeof(value_0)), testing::ElementsAreArray(value_0));
    ASSERT_THAT(SPAN_FROM_VALUE(a_Hashmap_Get(&hashmap, &key_1, sizeof(key_1)), sizeof(value_1)), testing::ElementsAreArray(value_1));

    value_0[0U] = 0x10U;
    value_1[0U] = 0x00U;
    a_Hashmap_Insert(&hashmap, &key_0, sizeof(key_0), value_0, sizeof(value_0));
    a_Hashmap_Insert(&hashmap, &key_1, sizeof(key_1), value_1, sizeof(value_1));
    ASSERT_THAT(SPAN_FROM_VALUE(a_Hashmap_Get(&hashmap, &key_0, sizeof(key_0)), sizeof(value_0)), testing::ElementsAreArray(value_0));
    ASSERT_THAT(SPAN_FROM_VALUE(a_Hashmap_Get(&hashmap, &key_1, sizeof(key_1)), sizeof(value_1)), testing::ElementsAreArray(value_1));

    a_Hashmap_Insert(&hashmap, &key_0, sizeof(key_0), value_0, sizeof(value_0) - 1U);
    ASSERT_THAT(SPAN_FROM_VALUE(a_Hashmap_Get(&hashmap, &key_0, sizeof(key_0)), sizeof(value_0) - 1U), testing::ElementsAreArray(value_0, sizeof(value_0) - 1U));

    /* TODO */
}

TEST_F(Hashmap, Remove)
{
    std::uint32_t key = 0x12345678U;
    std::uint8_t value[10U] = {0x00U, 0x01U, 0x02U, 0x03U, 0x04U, 0x05U, 0x06U, 0x07U, 0x08U, 0x09U};
    a_Hashmap_Initialize(&hashmap);

    ASSERT_EQ(A_ERR_NULL, a_Hashmap_Remove(nullptr, &key, sizeof(key)));
    ASSERT_EQ(A_ERR_NULL, a_Hashmap_Remove(&hashmap, nullptr, sizeof(key)));

    ASSERT_EQ(A_ERR_SIZE, a_Hashmap_Remove(&hashmap, &key, 0U));

    ASSERT_EQ(A_ERR_NONE, a_Hashmap_Remove(&hashmap, &key, sizeof(key)));

    a_Hashmap_Insert(&hashmap, &key, sizeof(key), value, sizeof(value));
    ASSERT_EQ(A_ERR_NONE, a_Hashmap_Remove(&hashmap, &key, sizeof(key)));
    ASSERT_EQ(nullptr, a_Hashmap_Get(&hashmap, &key, sizeof(key)));

    for (std::size_t i = 0U; i < 5; i++)
    {
        a_Hashmap_Insert(&hashmap, &key, sizeof(key), value, sizeof(value));
        key++;
    }
    for (std::size_t i = 0U; i < 5; i++)
    {
        key--;
        ASSERT_EQ(A_ERR_NONE, a_Hashmap_Remove(&hashmap, &key, sizeof(key)));
        ASSERT_EQ(nullptr, a_Hashmap_Get(&hashmap, &key, sizeof(key)));
    }

    /* TODO */
}

TEST_F(Hashmap, ForEach)
{
    const std::uint32_t key_0 = 0x12345678U;
    const std::uint32_t key_1 = 0x9ABCDEF0U;
    const std::uint32_t key_2 = 0xDEADBEEFU;
    const std::uint32_t key_3 = 0xDEAFBEADU;
    const std::uint32_t key_4 = 0xFEEDCEDEU;
    a_Hashmap_Initialize(&hashmap);

    {
        testing::InSequence sequence;

        EXPECT_CALL(*mock_callbacks_, Callback(testing::_, testing::_, testing::_, testing::_, testing::_))
            .Times(4)
            .WillRepeatedly([&](const void *const key, const std::size_t key_size, void *const value, const std::size_t value_size, const void *const arg)
                            {
                                A_UNUSED(key_size);
                                A_UNUSED(value);
                                A_UNUSED(value_size);
                                A_UNUSED(arg);

                                if (key_0 == *static_cast<const std::uint32_t *const>(key))
                                {
                                    (void)a_Hashmap_Remove(&hashmap, &key_1, sizeof(key_1));
                                }
                                else if (key_2 == *static_cast<const std::uint32_t *const>(key))
                                {
                                    (void)a_Hashmap_Remove(&hashmap, &key_0, sizeof(key_0));
                                }
                                else if (key_3 == *static_cast<const std::uint32_t *const>(key))
                                {
                                    a_Hashmap_Insert(&hashmap, &key_4, sizeof(key_4), &key_4, sizeof(key_4));
                                } });
    }

    ASSERT_EQ(A_ERR_NULL, a_Hashmap_ForEach(&hashmap, nullptr, nullptr));
    ASSERT_EQ(A_ERR_NULL, a_Hashmap_ForEach(nullptr, Callback, nullptr));

    ASSERT_EQ(A_ERR_NONE, a_Hashmap_ForEach(&hashmap, Callback, nullptr));

    a_Hashmap_Insert(&hashmap, &key_0, sizeof(key_0), &key_0, sizeof(key_0));
    a_Hashmap_Insert(&hashmap, &key_1, sizeof(key_1), &key_1, sizeof(key_1));
    a_Hashmap_Insert(&hashmap, &key_2, sizeof(key_2), &key_2, sizeof(key_2));
    a_Hashmap_Insert(&hashmap, &key_3, sizeof(key_3), &key_3, sizeof(key_3));
    ASSERT_EQ(A_ERR_NONE, a_Hashmap_ForEach(&hashmap, Callback, nullptr));
    ASSERT_EQ(nullptr, a_Hashmap_Get(&hashmap, &key_0, sizeof(key_0)));
    ASSERT_EQ(nullptr, a_Hashmap_Get(&hashmap, &key_1, sizeof(key_1)));
    ASSERT_EQ(key_2, *static_cast<std::uint32_t *>(a_Hashmap_Get(&hashmap, &key_2, sizeof(key_2))));
    ASSERT_EQ(key_3, *static_cast<std::uint32_t *>(a_Hashmap_Get(&hashmap, &key_3, sizeof(key_3))));
    ASSERT_EQ(key_4, *static_cast<std::uint32_t *>(a_Hashmap_Get(&hashmap, &key_4, sizeof(key_4))));

    /* TODO */
}
