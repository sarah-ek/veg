#include "static_assert.hpp"
#include <fmt/core.h>
#include <veg/tuple.hpp>
#include <utility>
#include <gtest/gtest.h>

#define MOV VEG_MOV

TEST(tuple, all) {
  using namespace veg;

  veg::tuple<int, char, bool> tup{1, 'c', true};
  veg::tuple<int const, char const, bool> tup_c{1, 'c', true};
  veg::tuple<int, char&&, char, bool&, bool const&> tup_ref{
      1, MOV(tup).as_ref()[1_c], 'c', fn::get<2>{}(tup), fn::get<2>{}(tup)};

  {
    STATIC_ASSERT(veg::meta::trivially_relocatable<decltype(tup)>::value);
    STATIC_ASSERT(veg::meta::trivially_copyable<decltype(tup)>::value);

    STATIC_ASSERT(veg::meta::trivially_copy_constructible<
                  veg::tuple<int&, int const&>>::value);
    STATIC_ASSERT(veg::meta::trivially_move_constructible<
                  veg::tuple<int&, int const&>>::value);
    STATIC_ASSERT(
        !veg::meta::copy_assignable<veg::tuple<int&, int const&>>::value);
    STATIC_ASSERT(
        !veg::meta::move_assignable<veg::tuple<int&, int const&>>::value);

    STATIC_ASSERT(veg::meta::copy_assignable<veg::tuple<int&, float&>>::value);
    STATIC_ASSERT(veg::meta::move_assignable<veg::tuple<int&, float&>>::value);

    STATIC_ASSERT(!veg::meta::copy_assignable<decltype(tup_ref)>::value);
    STATIC_ASSERT(!veg::meta::move_assignable<decltype(tup_ref)>::value);
    STATIC_ASSERT(!veg::meta::copy_constructible<decltype(tup_ref)>::value);
    STATIC_ASSERT(
        veg::meta::trivially_move_constructible<decltype(tup_ref)>::value);
    STATIC_ASSERT(!std::is_copy_constructible<decltype(tup_ref)>::value);
    STATIC_ASSERT(std::is_copy_constructible<decltype(tup)>::value);
    STATIC_ASSERT(std::is_copy_constructible<veg::tuple<int&, bool&>>::value);
    using val_tup = veg::tuple<int, bool>;
    using ref_tup = veg::tuple<int&, bool&>;

    STATIC_ASSERT(veg::meta::swappable<ref_tup&, ref_tup&>::value);
    STATIC_ASSERT(veg::meta::swappable<ref_tup&, ref_tup const&>::value);
    STATIC_ASSERT(veg::meta::swappable<ref_tup const&, ref_tup&>::value);
    STATIC_ASSERT(veg::meta::swappable<ref_tup const&, ref_tup const&>::value);
    STATIC_ASSERT(veg::meta::swappable<ref_tup&&, ref_tup&&>::value);
    STATIC_ASSERT(veg::meta::nothrow_swappable<ref_tup&&, ref_tup&&>::value);

    STATIC_ASSERT(veg::meta::swappable<ref_tup const&, val_tup&>::value);
    STATIC_ASSERT(veg::meta::swappable<val_tup&, val_tup&>::value);
    STATIC_ASSERT(!veg::meta::swappable<val_tup&&, val_tup&&>::value);
    STATIC_ASSERT(!veg::meta::swappable<val_tup&, val_tup&&>::value);
  }
  {
    using val_tup = veg::tuple<int, bool>;
    val_tup a{5, true};
    val_tup b{3, false};
    STATIC_ASSERT(
        veg::meta::constructible<veg::tuple<long, bool>, val_tup>::value);
    STATIC_ASSERT(
        veg::meta::constructible<veg::tuple<long, bool>, val_tup const&>::
            value);

    veg::swap(a, b);
    EXPECT_EQ(a[0_c], 3);
    EXPECT_EQ(b[0_c], 5);
    EXPECT_EQ(a[1_c], false);
    EXPECT_EQ(b[1_c], true);

    EXPECT_EQ(a, (val_tup{3, false}));
    EXPECT_EQ(b, (val_tup{5, true}));
  }
  {
    using ref_tup = veg::tuple<int&>;
    using val_tup = veg::tuple<int>;
    int i = 13;
    val_tup j{12};
    ref_tup const a{i};
    ref_tup const b{j};
    veg::swap(a, b);

    EXPECT_EQ(i, 12);
    EXPECT_EQ(j[0_c], 13);

    a = b;
    EXPECT_EQ(i, 13);
    EXPECT_EQ(j[0_c], 13);

    STATIC_ASSERT(veg::meta::assignable<ref_tup const&, ref_tup const&>::value);
  }

  {
    using ref_tup = veg::tuple<int&>;
    using rref_tup = veg::tuple<int&&>;
    STATIC_ASSERT(!veg::meta::trivially_copyable<ref_tup>::value);
    STATIC_ASSERT(!veg::meta::trivially_copyable<rref_tup>::value);
    STATIC_ASSERT(veg::meta::constructible<ref_tup, ref_tup>::value);
    STATIC_ASSERT(veg::meta::constructible<ref_tup, ref_tup&>::value);
    STATIC_ASSERT(veg::meta::constructible<ref_tup, rref_tup&>::value);
    STATIC_ASSERT(veg::meta::constructible<ref_tup, rref_tup const&>::value);
    STATIC_ASSERT(!veg::meta::constructible<ref_tup, rref_tup&&>::value);
    int i = 13;
    int j = 12;
    ref_tup a{i};
    rref_tup b{VEG_FWD(j)};

    veg::swap(a.as_ref(), b.as_ref());

    EXPECT_EQ(i, 12);
    EXPECT_EQ(j, 13);

    VEG_FWD(a) = VEG_FWD(b);
    static_cast<ref_tup const&>(a) = b;
    static_cast<ref_tup const&>(a) = VEG_FWD(b);
    EXPECT_EQ(i, 13);
    EXPECT_EQ(j, 13);

    STATIC_ASSERT(veg::meta::assignable<ref_tup const&, ref_tup const&>::value);
  }

  EXPECT_EQ(tup[0_c], 1);
  EXPECT_EQ(tup[1_c], 'c');
  EXPECT_TRUE(tup[2_c]);

  {
    auto&& ref = VEG_MOV(tup)[2_c];
    auto&& ref2 = VEG_MOV(tup).as_ref()[2_c];
    EXPECT_NE(&ref, &tup[2_c]);
    EXPECT_EQ(&ref2, &tup[2_c]);
  }

  VEG_BIND(auto, (e, f, g), [&] { return tup; }());
#if __cplusplus >= 201703L
  auto [i, c, b] = [&] { return tup; }();
  EXPECT_EQ(i, 1);
  EXPECT_EQ(c, 'c');
  EXPECT_TRUE(b);
  veg::tuple tup_deduce{1, 'c', true};
#endif

#define ASSERT_SAME(...) STATIC_ASSERT(::std::is_same<__VA_ARGS__>::value)

  STATIC_ASSERT(std::is_copy_assignable<veg::tuple<int, char>>());
  STATIC_ASSERT(std::is_trivially_copyable<veg::tuple<int, char>>());
  STATIC_ASSERT(std::is_copy_assignable<veg::tuple<int&, char&>>());
  STATIC_ASSERT(std::is_copy_assignable<veg::tuple<int&>>());
  STATIC_ASSERT(std::is_copy_assignable<veg::tuple<int&, char&> const>());
  STATIC_ASSERT(std::is_copy_assignable<veg::tuple<int&> const>());
  ASSERT_SAME(decltype(tup[0_c]), int&);
  ASSERT_SAME(decltype(tup[0_c]), decltype(tup[0_c]));
  ASSERT_SAME(decltype(MOV(tup)[0_c]), decltype(MOV(tup)[0_c]));
  ASSERT_SAME(decltype(tup_c[1_c]), char const&);
  ASSERT_SAME(decltype(MOV(tup)[0_c]), int);
  ASSERT_SAME(decltype(MOV(tup).as_ref()[0_c]), int&&);
  ASSERT_SAME(decltype(MOV(tup_c)[1_c]), char);

  ASSERT_SAME(decltype(tup.as_ref()), veg::tuple<int&, char&, bool&>);
  ASSERT_SAME(
      decltype(tup_c.as_ref()), veg::tuple<int const&, char const&, bool&>);
  ASSERT_SAME(decltype(MOV(tup).as_ref()), veg::tuple<int&&, char&&, bool&&>);

  ASSERT_SAME(decltype(e), int&&);
  ASSERT_SAME(decltype((e)), int&);
  ASSERT_SAME(decltype(f), char&&);
  ASSERT_SAME(decltype((f)), char&);
  ASSERT_SAME(decltype((g)), bool&);

  ASSERT_SAME(decltype(tup_ref[0_c]), int&);
  ASSERT_SAME(decltype(MOV(tup_ref).as_ref()[0_c]), int&&);

  ASSERT_SAME(decltype(tup_ref[1_c]), char&);
  ASSERT_SAME(decltype(tup_ref[2_c]), char&);
  ASSERT_SAME(decltype(MOV(tup_ref)[1_c]), char&&);
  ASSERT_SAME(decltype(MOV(tup_ref)[2_c]), char);
  ASSERT_SAME(decltype(MOV(tup_ref).as_ref()[2_c]), char&&);

  ASSERT_SAME(decltype(tup_ref[3_c]), bool&);
  ASSERT_SAME(decltype(tup_ref[4_c]), bool const&);
  ASSERT_SAME(decltype(MOV(tup_ref)[3_c]), bool&);
  ASSERT_SAME(decltype(MOV(tup_ref)[4_c]), bool const&);

#if __cplusplus >= 201703L
  ASSERT_SAME(decltype(i), int);
  ASSERT_SAME(decltype((i)), int&);
  ASSERT_SAME(decltype((b)), bool&);
  ASSERT_SAME(decltype(tup_deduce), veg::tuple<int, char, bool>);
#endif

  STATIC_ASSERT(tuple<int>{1} == tuple<int>{1});
  STATIC_ASSERT(tuple<int>{1} == tuple<float>{1});
  STATIC_ASSERT(tuple<int>{1} != tuple<float>{2});
  STATIC_ASSERT(tuple<int, float>{1, 2.0F} == tuple<float, int>{1.0F, 2});
  STATIC_ASSERT(tuple<int, float>{1, 2.0F} == tuple<float, int>{1.0F, 2});
  STATIC_ASSERT(tuple<int, float>{1, 2.0F} != tuple<float, int>{2.0F, 2});
}

TEST(tuple, nested) {
  using namespace veg;
  tuple<int, tuple<int, float>> tup{1, {elems, 2, 3.0F}};
  ASSERT_EQ(tup[0_c], 1);

  ASSERT_EQ(tup[1_c][1_c], 3.0F);
  ASSERT_EQ(tup[1_c][0_c], 2);
  ASSERT_SAME(decltype(tup[1_c][0_c]), int&);
  ASSERT_SAME(decltype(tup[1_c][0_c]), int&);
  ASSERT_SAME(decltype(VEG_MOV(tup)[1_c][0_c]), int);
}

TEST(tuple, empty) {
  using namespace veg;
  tuple<> t1;
  tuple<> t2(inplace);
  EXPECT_EQ(t1, t2);
  STATIC_ASSERT(tuple<>{} == tuple<>{});
}

TEST(tuple, cvt) {
  using namespace veg;
  tuple<int, float> t1{1, 1.5F};
  tuple<long, double> t2(3, 2.5);
  tuple<long, double> t3(t1);
  tuple<int, float> t4(t3);

  EXPECT_EQ(t1, t4);
  t1 = t2;
  EXPECT_EQ(t1, t2);
}

TEST(tuple, non_movable) {
  struct S {
    S() = default;
    S(S&&) = delete;
    S(S const&) = delete;
  };
  using namespace veg;
  {
    tuple<S> s{};
    (void)s;
  }
  __VEG_CPP17({
    tuple<S> s{inplace, [] { return S{}; }};
    (void)s;
  })
}

TEST(tuple, get) {
  using namespace veg;
  int arr[] = {1, 2, 3};
  veg::fn::get<0>{}(arr);
  veg::fn::get<0>{}(VEG_FWD(arr));
}
