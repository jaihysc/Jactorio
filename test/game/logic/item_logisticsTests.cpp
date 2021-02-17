// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "jactorioTests.h"

#include "game/logic/inserter_controller.h"
#include "game/logic/item_logistics.h"
#include "proto/container_entity.h"
#include "proto/inserter.h"
#include "proto/mining_drill.h"
#include "proto/transport_belt.h"

namespace jactorio::game
{
    class ItemLogisticsTest : public testing::Test
    {
    protected:
        World world_;
        Logic logic_;

        void SetUp() override {
            world_.EmplaceChunk({0, 0});
        }
    };

    TEST_F(ItemLogisticsTest, Uninitialize) {
        ItemDropOff drop_off{Orientation::up};

        auto& layer = world_.GetTile({2, 4})->Entity();

        proto::ContainerEntity container;
        layer.SetPrototype(Orientation::up, &container);
        layer.MakeUniqueData<proto::ContainerEntityData>(1);

        ASSERT_TRUE(drop_off.Initialize(world_, {2, 4}));
        drop_off.Uninitialize();

        EXPECT_FALSE(drop_off.IsInitialized());
    }

    TEST_F(ItemLogisticsTest, DropOffDropItem) {
        proto::ContainerEntity container_entity;
        auto& container_layer = TestSetupContainer(world_, {2, 4}, Orientation::up, container_entity);

        ItemDropOff drop_off{Orientation::up};
        ASSERT_TRUE(drop_off.Initialize(world_, {2, 4}));

        proto::Item item;
        drop_off.DropOff(logic_, {&item, 10});

        EXPECT_EQ(container_layer.GetUniqueData<proto::ContainerEntityData>()->inventory[0].count, 10);
    }

    TEST_F(ItemLogisticsTest, InserterPickupItem) {
        proto::ContainerEntity container_entity;
        auto& container_layer = TestSetupContainer(world_, {2, 4}, Orientation::up, container_entity);

        InserterPickup pickup{Orientation::up};
        ASSERT_TRUE(pickup.Initialize(world_, {2, 4}));

        proto::Item item;
        auto& inv = container_layer.GetUniqueData<proto::ContainerEntityData>()->inventory;
        inv[0]    = {&item, 10};

        pickup.Pickup(logic_, 1, proto::RotationDegreeT(180.f), 2);

        EXPECT_EQ(inv[0].count, 8);
    }

    // ======================================================================

    ///
    /// Inherits ItemDropOff to gain access to insertion methods
    class ItemDropOffTest : public testing::Test, public ItemDropOff
    {
    public:
        void SetUp() override {
            world_.EmplaceChunk({0, 0});
        }

        explicit ItemDropOffTest() : ItemDropOff(Orientation::up) {}

    protected:
        World world_;
        Logic logic_;

        /// Creates a conveyor with orientation
        proto::ConveyorData CreateConveyor(
            const Orientation orientation,
            const ConveyorStruct::TerminationType ttype = ConveyorStruct::TerminationType::straight) const {
            const auto segment = std::make_shared<ConveyorStruct>(orientation, ttype, 2);

            return proto::ConveyorData{segment};
        }

        ///
        /// \param orientation Inserter orientation to dropoff
        void ConveyorInsert(const Orientation orientation, proto::ConveyorData& line_data) {
            proto::Item item;
            InsertTransportBelt({logic_, {&item, 1}, line_data, orientation});

            EXPECT_TRUE(CanInsertTransportBelt({logic_, {}, line_data, Orientation::up}));
        }
    };

    TEST_F(ItemDropOffTest, GetInsertFunc) {
        world_.GetTile({2, 4})->Entity().MakeUniqueData<proto::ContainerEntityData>(1);

        auto set_prototype = [&](proto::Entity& entity_proto) {
            auto& layer = world_.GetTile({2, 4})->Entity();
            layer.SetPrototype(Orientation::up, &entity_proto);
        };

        // No: Empty tile cannot be inserted into
        EXPECT_FALSE(this->Initialize(world_, {2, 4}));
        EXPECT_FALSE(this->IsInitialized());


        // Ok: Transport belt can be inserted onto
        proto::TransportBelt belt;
        set_prototype(belt);

        EXPECT_TRUE(this->Initialize(world_, {2, 4}));
        EXPECT_TRUE(this->IsInitialized());


        // No: Mining drill
        proto::MiningDrill drill;
        set_prototype(drill);

        EXPECT_FALSE(this->Initialize(world_, {2, 4}));
        EXPECT_TRUE(this->IsInitialized()); // Still initialized from conveyor belt


        // Ok: Container
        proto::ContainerEntity container;
        set_prototype(container);

        EXPECT_TRUE(this->Initialize(world_, {2, 4}));
        EXPECT_TRUE(this->IsInitialized());


        // Ok: Assembly machine
        proto::AssemblyMachine assembly_machine; // Will also make unique data, so it needs to be on another tile
        assembly_machine.SetDimensions(2, 2);
        TestSetupAssemblyMachine(world_, {3, 4}, Orientation::up, assembly_machine);

        EXPECT_TRUE(this->Initialize(world_, {3, 5}));
        EXPECT_TRUE(this->IsInitialized());
        EXPECT_EQ(this->targetProtoData_, &assembly_machine);
    }

    // ======================================================================

    TEST_F(ItemDropOffTest, InsertContainerEntity) {
        auto& layer = world_.GetTile({3, 1})->Entity();

        layer.MakeUniqueData<proto::ContainerEntityData>(10);

        auto* container_data = layer.GetUniqueData<proto::ContainerEntityData>();


        proto::Item item;
        // Orientation is orientation from origin object
        EXPECT_TRUE(InsertContainerEntity({logic_, {&item, 2}, *container_data, Orientation::down}));
        EXPECT_TRUE(CanInsertContainerEntity({logic_, {}, *container_data, Orientation::up}));

        // Inserted item
        EXPECT_EQ(container_data->inventory[0].item, &item);
        EXPECT_EQ(container_data->inventory[0].count, 2);
    }

    TEST_F(ItemDropOffTest, InsertOffset) {
        auto line_data                  = CreateConveyor(Orientation::up);
        line_data.structIndex           = 1;
        line_data.structure->headOffset = 10; // Arbitrary

        ConveyorInsert(Orientation::up, line_data);
        ASSERT_EQ(line_data.structure->right.lane.size(), 1);
        EXPECT_DOUBLE_EQ(line_data.structure->right.lane[0].dist.getAsDouble(), 1.5);
    }

    TEST_F(ItemDropOffTest, InsertConveyorUp) {
        {
            auto line = CreateConveyor(Orientation::up);

            ConveyorInsert(Orientation::up, line);
            EXPECT_EQ(line.structure->right.lane.size(), 1);
        }
        {
            auto line = CreateConveyor(Orientation::up);

            ConveyorInsert(Orientation::right, line);
            EXPECT_EQ(line.structure->left.lane.size(), 1);
        }
        {
            auto line = CreateConveyor(Orientation::up);

            ConveyorInsert(Orientation::down, line);
            EXPECT_EQ(line.structure->left.lane.size(), 0);
            EXPECT_EQ(line.structure->right.lane.size(), 0);
        }
        {
            auto line = CreateConveyor(Orientation::up);

            ConveyorInsert(Orientation::left, line);
            EXPECT_EQ(line.structure->right.lane.size(), 1);
        }
    }

    TEST_F(ItemDropOffTest, InsertConveyorRight) {
        {
            auto line = CreateConveyor(Orientation::right);

            ConveyorInsert(Orientation::up, line);
            EXPECT_EQ(line.structure->right.lane.size(), 1);
        }
        {
            auto line = CreateConveyor(Orientation::right);

            ConveyorInsert(Orientation::right, line);
            EXPECT_EQ(line.structure->right.lane.size(), 1);
        }
        {
            auto line = CreateConveyor(Orientation::right);

            ConveyorInsert(Orientation::down, line);
            EXPECT_EQ(line.structure->left.lane.size(), 1);
        }
        {
            auto line = CreateConveyor(Orientation::right);

            ConveyorInsert(Orientation::left, line);
            EXPECT_EQ(line.structure->left.lane.size(), 0);
            EXPECT_EQ(line.structure->right.lane.size(), 0);
        }
    }

    TEST_F(ItemDropOffTest, InsertConveyorDown) {
        {
            auto line = CreateConveyor(Orientation::down);

            ConveyorInsert(Orientation::up, line);
            EXPECT_EQ(line.structure->left.lane.size(), 0);
            EXPECT_EQ(line.structure->right.lane.size(), 0);
        }
        {
            auto line = CreateConveyor(Orientation::down);

            ConveyorInsert(Orientation::right, line);
            EXPECT_EQ(line.structure->right.lane.size(), 1);
        }
        {
            auto line = CreateConveyor(Orientation::down);

            ConveyorInsert(Orientation::down, line);
            EXPECT_EQ(line.structure->right.lane.size(), 1);
        }
        {
            auto line = CreateConveyor(Orientation::down);

            ConveyorInsert(Orientation::left, line);
            EXPECT_EQ(line.structure->left.lane.size(), 1);
        }
    }

    TEST_F(ItemDropOffTest, InsertConveyorLeft) {
        {
            auto line = CreateConveyor(Orientation::left);

            ConveyorInsert(Orientation::up, line);
            EXPECT_EQ(line.structure->left.lane.size(), 1);
        }
        {
            auto line = CreateConveyor(Orientation::left);

            ConveyorInsert(Orientation::right, line);
            EXPECT_EQ(line.structure->left.lane.size(), 0);
            EXPECT_EQ(line.structure->right.lane.size(), 0);
        }
        {
            auto line = CreateConveyor(Orientation::left);

            ConveyorInsert(Orientation::down, line);
            EXPECT_EQ(line.structure->right.lane.size(), 1);
        }
        {
            auto line = CreateConveyor(Orientation::left);

            ConveyorInsert(Orientation::left, line);
            EXPECT_EQ(line.structure->right.lane.size(), 1);
        }
    }

    TEST_F(ItemDropOffTest, DropoffConveyorNonStraight) {
        // -->
        // ^
        // | <--

        const auto right = CreateConveyor(Orientation::right, ConveyorStruct::TerminationType::straight);
        const auto up    = CreateConveyor(Orientation::up, ConveyorStruct::TerminationType::bend_right);
        auto left        = CreateConveyor(Orientation::left, ConveyorStruct::TerminationType::bend_right);

        left.structure->target = up.structure.get();
        up.structure->target   = right.structure.get();

        left.structIndex = 1;


        proto::Item item;

        ConveyorInsert(Orientation::up, left);
        ASSERT_EQ(left.structure->left.lane.size(), 1);
        EXPECT_DOUBLE_EQ(left.structure->left.lane[0].dist.getAsDouble(), 0.5 + 0.7);
    }

    TEST_F(ItemDropOffTest, InsertAssemblyMachine) {
        data::PrototypeManager proto;

        auto recipe_pack = TestSetupRecipe(proto);

        proto::AssemblyMachineData asm_data;
        asm_data.ChangeRecipe(logic_, proto, recipe_pack.recipe);

        asm_data.ingredientInv[0] = {recipe_pack.item1, 5, recipe_pack.item1};
        asm_data.ingredientInv[1] = {nullptr, 0, recipe_pack.item2};


        // Needs prototype data to register crafting callback
        proto::AssemblyMachine assembly_machine;
        targetProtoData_ = &assembly_machine;

        // Orientation doesn't matter
        EXPECT_TRUE(InsertAssemblyMachine({logic_, {recipe_pack.item2, 10}, asm_data, Orientation::up}));

        EXPECT_EQ(asm_data.ingredientInv[1].item, recipe_pack.item2);
        EXPECT_EQ(asm_data.ingredientInv[1].count, 9); // 1 used to begin crafting

        // Registered to start crafting
        EXPECT_NE(asm_data.deferralEntry.callbackIndex, 0);
    }

    TEST_F(ItemDropOffTest, InsertAssemblyMachineExceedStack) {
        // Inserters will not insert into assembly machines if it will exceed the current item's stack limit

        data::PrototypeManager proto;
        const auto recipe_pack = TestSetupRecipe(proto);

        proto::AssemblyMachineData asm_data;
        asm_data.ingredientInv.Resize(2);
        asm_data.ingredientInv[0] = {recipe_pack.item1, 49, recipe_pack.item1};
        asm_data.ingredientInv[1] = {nullptr, 0, recipe_pack.item2};

        // 49 + 2 > 50
        EXPECT_FALSE(InsertAssemblyMachine({logic_, {recipe_pack.item1, 2}, asm_data, Orientation::up}));
    }

    TEST_F(ItemDropOffTest, CanInsertAssemblyMachine) {
        proto::AssemblyMachineData asm_data;

        proto::Item item;

        // No recipe
        EXPECT_FALSE(CanInsertAssemblyMachine({logic_, {&item, 2}, asm_data, Orientation::down}));


        data::PrototypeManager proto;

        auto recipe_pack = TestSetupRecipe(proto);
        asm_data.ChangeRecipe(logic_, proto, recipe_pack.recipe);

        // Has recipe, wrong item
        EXPECT_FALSE(CanInsertAssemblyMachine({logic_, {&item, 2}, asm_data, Orientation::down}));

        // Has recipe, correct item
        DropOffParams args{logic_, {recipe_pack.item1, 2000}, asm_data, Orientation::down};
        EXPECT_TRUE(CanInsertAssemblyMachine(args));

        // Exceeds the max ingredient count multiple that a slot can be filled
        asm_data.ingredientInv[0].count = 2;
        EXPECT_FALSE(CanInsertAssemblyMachine(args));

        // Exceeds max stack size
        recipe_pack.recipe->ingredients[0].second = 50; // Requires 50, can only hold 50
        recipe_pack.item1->stackSize              = 50;
        asm_data.ingredientInv[0].count           = 50;
        EXPECT_FALSE(CanInsertAssemblyMachine(args));

        // Under the max ingredient count multiple that a slot can be filled
        asm_data.ingredientInv[0].count = 1;
        EXPECT_TRUE(CanInsertAssemblyMachine(args));
    }

    // ======================================================================

    class InserterPickupTest : public testing::Test, public InserterPickup
    {
    public:
        explicit InserterPickupTest() : InserterPickup(Orientation::up) {}

        void SetUp() override {
            world_.EmplaceChunk({0, 0});
        }

    protected:
        World world_;
        Logic logic_;

        proto::Inserter inserterProto_;

        /// Item which will be on conveyor segments from CreateConveyor
        proto::Item lineItem_;

        /// Creates a conveyor with  1 item on each side
        proto::ConveyorData CreateConveyor(const Orientation orientation) const {

            const auto segment =
                std::make_shared<ConveyorStruct>(orientation, ConveyorStruct::TerminationType::straight, 2);

            segment->InsertItem(false, 0.5, lineItem_);
            segment->InsertItem(true, 0.5, lineItem_);

            return proto::ConveyorData{segment};
        }


        /// \param orientation Orientation to line being picked up from
        void PickupLine(const Orientation orientation, proto::ConveyorData& line_data) {
            constexpr int pickup_amount = 1;

            const auto result = PickupTransportBelt(
                {logic_, 1, proto::RotationDegreeT(kMaxInserterDegree), pickup_amount, line_data, orientation});

            EXPECT_TRUE(result.first);

            EXPECT_NE(result.second.item, nullptr);
            EXPECT_EQ(result.second.count, pickup_amount);
        }
    };

    TEST_F(InserterPickupTest, GetPickupFunc) {
        world_.GetTile({2, 4})->Entity().MakeUniqueData<proto::ContainerEntityData>(1);

        auto set_prototype = [&](proto::Entity& entity_proto) {
            auto& layer = world_.GetTile({2, 4})->Entity();
            layer.SetPrototype(Orientation::up, &entity_proto);
        };


        // No: Empty tile cannot be picked up from
        EXPECT_FALSE(this->Initialize(world_, {2, 4}));
        EXPECT_FALSE(this->IsInitialized());


        // Ok: Transport belt can be picked up from
        proto::TransportBelt belt;
        set_prototype(belt);

        EXPECT_TRUE(this->Initialize(world_, {2, 4}));
        EXPECT_TRUE(this->IsInitialized());


        // No: Mining drill
        proto::MiningDrill drill;
        set_prototype(drill);

        EXPECT_FALSE(this->Initialize(world_, {2, 4}));
        EXPECT_TRUE(this->IsInitialized()); // Still initialized from conveyor belt


        // Ok: Container
        proto::ContainerEntity container;
        set_prototype(container);

        EXPECT_TRUE(this->Initialize(world_, {2, 4}));
        EXPECT_TRUE(this->IsInitialized());


        // Ok: Assembly machine
        proto::AssemblyMachine assembly_machine;
        assembly_machine.SetDimensions(2, 2);
        TestSetupAssemblyMachine(world_, {3, 4}, Orientation::up, assembly_machine);

        EXPECT_TRUE(this->Initialize(world_, {4, 5}));
        EXPECT_TRUE(this->IsInitialized());
        EXPECT_EQ(this->targetProtoData_, &assembly_machine);
    }

    TEST_F(InserterPickupTest, PickupContainerEntity) {
        proto::ContainerEntity container_entity;
        auto& container_layer = TestSetupContainer(world_, {2, 4}, Orientation::up, container_entity);
        auto& container_data  = *container_layer.GetUniqueData();

        proto::Item item;
        auto& inv = container_layer.GetUniqueData<proto::ContainerEntityData>()->inventory;
        inv[0]    = {&item, 10};


        PickupContainerEntity({logic_, 1, proto::RotationDegreeT(179), 1, container_data, Orientation::up});
        EXPECT_EQ(inv[0].count, 10); // No items picked up, not 180 degrees


        PickupParams args = {logic_, 1, proto::RotationDegreeT(180), 2, container_data, Orientation::up};

        EXPECT_EQ(GetPickupContainerEntity(args), &item);

        auto result = PickupContainerEntity(args);
        EXPECT_EQ(inv[0].count, 8); // At 180 degrees, 2 items picked up
        EXPECT_EQ(result.second.item, &item);
        EXPECT_EQ(result.second.count, 2);
    }

    TEST_F(InserterPickupTest, PickupConveyorUp) {
        // Line is above inserter
        {
            auto line = CreateConveyor(Orientation::up);

            PickupLine(Orientation::up, line);
            EXPECT_EQ(line.structure->right.lane.size(), 0);
        }
        {
            auto line = CreateConveyor(Orientation::up);

            PickupLine(Orientation::right, line);
            EXPECT_EQ(line.structure->right.lane.size(), 0);
        }
        {
            auto line = CreateConveyor(Orientation::up);

            PickupLine(Orientation::down, line);
            EXPECT_EQ(line.structure->left.lane.size(), 0);
        }
        {
            auto line = CreateConveyor(Orientation::up);

            PickupLine(Orientation::left, line);
            EXPECT_EQ(line.structure->left.lane.size(), 0);
        }
    }

    TEST_F(InserterPickupTest, PickupConveyorRight) {
        {
            auto line = CreateConveyor(Orientation::right);

            PickupLine(Orientation::up, line);
            EXPECT_EQ(line.structure->left.lane.size(), 0);
        }
        {
            auto line = CreateConveyor(Orientation::right);

            PickupLine(Orientation::right, line);
            EXPECT_EQ(line.structure->right.lane.size(), 0);
        }
        {
            auto line = CreateConveyor(Orientation::right);

            PickupLine(Orientation::down, line);
            EXPECT_EQ(line.structure->right.lane.size(), 0);
        }
        {
            auto line = CreateConveyor(Orientation::right);

            PickupLine(Orientation::left, line);
            EXPECT_EQ(line.structure->left.lane.size(), 0);
        }
    }

    TEST_F(InserterPickupTest, PickupConveyorDown) {
        {
            auto line = CreateConveyor(Orientation::down);

            PickupLine(Orientation::up, line);
            EXPECT_EQ(line.structure->left.lane.size(), 0);
        }
        {
            auto line = CreateConveyor(Orientation::down);

            PickupLine(Orientation::right, line);
            EXPECT_EQ(line.structure->left.lane.size(), 0);
        }
        {
            auto line = CreateConveyor(Orientation::down);

            PickupLine(Orientation::down, line);
            EXPECT_EQ(line.structure->right.lane.size(), 0);
        }
        {
            auto line = CreateConveyor(Orientation::down);

            PickupLine(Orientation::left, line);
            EXPECT_EQ(line.structure->right.lane.size(), 0);
        }
    }

    TEST_F(InserterPickupTest, PickupConveyorLeft) {
        {
            auto line = CreateConveyor(Orientation::left);

            PickupLine(Orientation::up, line);
            EXPECT_EQ(line.structure->right.lane.size(), 0);
        }
        {
            auto line = CreateConveyor(Orientation::left);

            PickupLine(Orientation::right, line);
            EXPECT_EQ(line.structure->left.lane.size(), 0);
        }
        {
            auto line = CreateConveyor(Orientation::left);

            PickupLine(Orientation::down, line);
            EXPECT_EQ(line.structure->left.lane.size(), 0);
        }
        {
            auto line = CreateConveyor(Orientation::left);

            PickupLine(Orientation::left, line);
            EXPECT_EQ(line.structure->right.lane.size(), 0);
        }
    }

    TEST_F(InserterPickupTest, PickupConveyorNonStraight) {
        // -->
        // ^
        // | <--

        const auto right =
            std::make_shared<ConveyorStruct>(Orientation::right, ConveyorStruct::TerminationType::straight, 2);

        const auto up =
            std::make_shared<ConveyorStruct>(Orientation::up, ConveyorStruct::TerminationType::bend_right, 2);

        const auto left =
            std::make_shared<ConveyorStruct>(Orientation::left, ConveyorStruct::TerminationType::bend_right, 2);

        left->target = up.get();
        up->target   = right.get();

        proto::ConveyorData line{left};
        line.structIndex = 1;


        const proto::Item item;

        left->AppendItem(true, 0.5 + 0.7, item);
        PickupLine(Orientation::up, line);
        EXPECT_EQ(left->left.lane.size(), 0);

        left->AppendItem(false, 0.5 + 0.3, item);
        PickupLine(Orientation::up, line);
        EXPECT_EQ(left->right.lane.size(), 0);
    }

    TEST_F(InserterPickupTest, PickupConveyorAlternativeSide) {
        // If the preferred lane is available, inserter will attempt to pick up from other lane
        auto line = CreateConveyor(Orientation::up);

        EXPECT_EQ(GetPickupTransportBelt({logic_, 1, proto::RotationDegreeT(180), 1, line, Orientation::up}),
                  &lineItem_);

        // Should set index to 0 since a item was removed
        line.structure->right.index = 10;

        PickupLine(Orientation::up, line);
        EXPECT_EQ(line.structure->right.lane.size(), 0);
        EXPECT_EQ(line.structure->right.index, 0);


        // Use alternative side
        EXPECT_EQ(GetPickupTransportBelt({logic_, 1, proto::RotationDegreeT(180), 1, line, Orientation::up}),
                  &lineItem_);

        line.structure->left.index = 10;

        PickupLine(Orientation::up, line);
        EXPECT_EQ(line.structure->left.lane.size(), 0);
        EXPECT_EQ(line.structure->left.index, 0);
    }


    TEST_F(InserterPickupTest, PickupAssemblyMachine) {
        data::PrototypeManager proto;

        proto::AssemblyMachine asm_machine;
        auto& layer = TestSetupAssemblyMachine(world_, {0, 0}, Orientation::up, asm_machine);
        auto* data  = layer.GetUniqueData<proto::AssemblyMachineData>();

        // Does nothing as there is no recipe yet
        PickupAssemblyMachine({logic_, 2, proto::RotationDegreeT(kMaxInserterDegree), 2, *data, Orientation::up});

        // ======================================================================

        auto recipe_pack = TestSetupRecipe(proto);


        data->ChangeRecipe(logic_, proto, recipe_pack.recipe);

        // No items in product inventory
        EXPECT_FALSE(
            PickupAssemblyMachine({logic_, 2, proto::RotationDegreeT(kMaxInserterDegree), 2, *data, Orientation::up})
                .first);


        // Has items in product inventory
        data->ingredientInv[0] = {recipe_pack.item1, 1};
        data->ingredientInv[1] = {recipe_pack.item2, 1};

        data->productInv[0] = {recipe_pack.itemProduct, 10, recipe_pack.itemProduct};

        targetProtoData_ = &asm_machine;

        PickupParams args{logic_, 2, proto::RotationDegreeT(kMaxInserterDegree), 2, *data, Orientation::up};

        EXPECT_EQ(GetPickupAssemblyMachine(args), recipe_pack.itemProduct);
        auto result = PickupAssemblyMachine(args);

        EXPECT_TRUE(result.first);
        EXPECT_EQ(result.second.item, recipe_pack.itemProduct);
        EXPECT_EQ(result.second.count, 2);

        EXPECT_EQ(data->productInv[0].count, 8);

        // Begin crafting since ingredients are met
        EXPECT_NE(data->deferralEntry.callbackIndex, 0);
    }
} // namespace jactorio::game
