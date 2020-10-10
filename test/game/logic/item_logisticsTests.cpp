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
        WorldData worldData_;
        LogicData logicData_;

        void SetUp() override {
            worldData_.EmplaceChunk(0, 0);
        }
    };

    TEST_F(ItemLogisticsTest, Uninitialize) {
        ItemDropOff drop_off{proto::Orientation::up};

        auto& layer = worldData_.GetTile(2, 4)->GetLayer(TileLayer::entity);

        proto::ContainerEntity container;
        layer.prototypeData = &container;
        layer.MakeUniqueData<proto::ContainerEntityData>(1);

        ASSERT_TRUE(drop_off.Initialize(worldData_, {2, 4}));
        drop_off.Uninitialize();

        EXPECT_FALSE(drop_off.IsInitialized());
    }

    TEST_F(ItemLogisticsTest, DropOffDropItem) {
        proto::ContainerEntity container_entity;
        auto& container_layer = TestSetupContainer(worldData_, {2, 4}, container_entity);

        ItemDropOff drop_off{proto::Orientation::up};
        ASSERT_TRUE(drop_off.Initialize(worldData_, {2, 4}));

        proto::Item item;
        drop_off.DropOff(logicData_, {&item, 10});

        EXPECT_EQ(container_layer.GetUniqueData<proto::ContainerEntityData>()->inventory[0].count, 10);
    }

    TEST_F(ItemLogisticsTest, InserterPickupItem) {
        proto::ContainerEntity container_entity;
        auto& container_layer = TestSetupContainer(worldData_, {2, 4}, container_entity);

        InserterPickup pickup{proto::Orientation::up};
        ASSERT_TRUE(pickup.Initialize(worldData_, {2, 4}));

        proto::Item item;
        auto& inv = container_layer.GetUniqueData<proto::ContainerEntityData>()->inventory;
        inv[0]    = {&item, 10};

        pickup.Pickup(logicData_, 1, proto::RotationDegreeT(180.f), 2);

        EXPECT_EQ(inv[0].count, 8);
    }

    // ======================================================================

    ///
    /// Inherits ItemDropOff to gain access to insertion methods
    class ItemDropOffTest : public testing::Test, public ItemDropOff
    {
    public:
        void SetUp() override {
            worldData_.EmplaceChunk(0, 0);
        }

        explicit ItemDropOffTest() : ItemDropOff(proto::Orientation::up) {}

    protected:
        WorldData worldData_;
        LogicData logicData_;

        /// Creates a conveyor with orientation
        proto::ConveyorData CreateConveyor(
            const proto::Orientation orientation,
            const ConveyorSegment::TerminationType ttype = ConveyorSegment::TerminationType::straight) const {
            const auto segment = std::make_shared<ConveyorSegment>(orientation, ttype, 2);

            return proto::ConveyorData{segment};
        }

        ///
        /// \param orientation Inserter orientation to dropoff
        void ConveyorInsert(const proto::Orientation orientation, proto::ConveyorData& line_data) {
            proto::Item item;
            InsertTransportBelt({logicData_, {&item, 1}, line_data, orientation});

            EXPECT_TRUE(CanInsertTransportBelt({logicData_, {}, line_data, proto::Orientation::up}));
        }
    };

    TEST_F(ItemDropOffTest, GetInsertFunc) {
        worldData_.GetTile(2, 4)->GetLayer(TileLayer::entity).MakeUniqueData<proto::ContainerEntityData>(1);

        auto set_prototype = [&](proto::Entity& entity_proto) {
            auto& layer         = worldData_.GetTile(2, 4)->GetLayer(TileLayer::entity);
            layer.prototypeData = &entity_proto;
        };

        // No: Empty tile cannot be inserted into
        EXPECT_FALSE(this->Initialize(worldData_, {2, 4}));
        EXPECT_FALSE(this->IsInitialized());


        // Ok: Transport belt can be inserted onto
        proto::TransportBelt belt;
        set_prototype(belt);

        EXPECT_TRUE(this->Initialize(worldData_, {2, 4}));
        EXPECT_TRUE(this->IsInitialized());


        // No: Mining drill
        proto::MiningDrill drill;
        set_prototype(drill);

        EXPECT_FALSE(this->Initialize(worldData_, {2, 4}));
        EXPECT_TRUE(this->IsInitialized()); // Still initialized from conveyor belt


        // Ok: Container
        proto::ContainerEntity container;
        set_prototype(container);

        EXPECT_TRUE(this->Initialize(worldData_, {2, 4}));
        EXPECT_TRUE(this->IsInitialized());


        // Ok: Assembly machine
        proto::AssemblyMachine assembly_machine; // Will also make unique data, so it needs to be on another tile
        TestSetupAssemblyMachine(worldData_, {3, 4}, assembly_machine);

        EXPECT_TRUE(this->Initialize(worldData_, {3, 5}));
        EXPECT_TRUE(this->IsInitialized());
        EXPECT_EQ(this->targetProtoData_, &assembly_machine);
    }

    // ======================================================================

    TEST_F(ItemDropOffTest, InsertContainerEntity) {
        auto& layer = worldData_.GetTile(3, 1)->GetLayer(TileLayer::entity);

        layer.MakeUniqueData<proto::ContainerEntityData>(10);

        auto* container_data = layer.GetUniqueData<proto::ContainerEntityData>();


        proto::Item item;
        // Orientation is orientation from origin object
        EXPECT_TRUE(InsertContainerEntity({logicData_, {&item, 2}, *container_data, proto::Orientation::down}));
        EXPECT_TRUE(CanInsertContainerEntity({logicData_, {}, *container_data, proto::Orientation::up}));

        // Inserted item
        EXPECT_EQ(container_data->inventory[0].item, &item);
        EXPECT_EQ(container_data->inventory[0].count, 2);
    }

    TEST_F(ItemDropOffTest, InsertOffset) {
        auto line_data                    = CreateConveyor(proto::Orientation::up);
        line_data.lineSegmentIndex        = 1;
        line_data.lineSegment->itemOffset = 10; // Arbitrary itemOffset

        ConveyorInsert(proto::Orientation::up, line_data);
        ASSERT_EQ(line_data.lineSegment->right.lane.size(), 1);
        EXPECT_DOUBLE_EQ(line_data.lineSegment->right.lane[0].dist.getAsDouble(), 1.5);
    }

    TEST_F(ItemDropOffTest, InsertConveyorUp) {
        {
            auto line = CreateConveyor(proto::Orientation::up);

            ConveyorInsert(proto::Orientation::up, line);
            EXPECT_EQ(line.lineSegment->right.lane.size(), 1);
        }
        {
            auto line = CreateConveyor(proto::Orientation::up);

            ConveyorInsert(proto::Orientation::right, line);
            EXPECT_EQ(line.lineSegment->left.lane.size(), 1);
        }
        {
            auto line = CreateConveyor(proto::Orientation::up);

            ConveyorInsert(proto::Orientation::down, line);
            EXPECT_EQ(line.lineSegment->left.lane.size(), 0);
            EXPECT_EQ(line.lineSegment->right.lane.size(), 0);
        }
        {
            auto line = CreateConveyor(proto::Orientation::up);

            ConveyorInsert(proto::Orientation::left, line);
            EXPECT_EQ(line.lineSegment->right.lane.size(), 1);
        }
    }

    TEST_F(ItemDropOffTest, InsertConveyorRight) {
        {
            auto line = CreateConveyor(proto::Orientation::right);

            ConveyorInsert(proto::Orientation::up, line);
            EXPECT_EQ(line.lineSegment->right.lane.size(), 1);
        }
        {
            auto line = CreateConveyor(proto::Orientation::right);

            ConveyorInsert(proto::Orientation::right, line);
            EXPECT_EQ(line.lineSegment->right.lane.size(), 1);
        }
        {
            auto line = CreateConveyor(proto::Orientation::right);

            ConveyorInsert(proto::Orientation::down, line);
            EXPECT_EQ(line.lineSegment->left.lane.size(), 1);
        }
        {
            auto line = CreateConveyor(proto::Orientation::right);

            ConveyorInsert(proto::Orientation::left, line);
            EXPECT_EQ(line.lineSegment->left.lane.size(), 0);
            EXPECT_EQ(line.lineSegment->right.lane.size(), 0);
        }
    }

    TEST_F(ItemDropOffTest, InsertConveyorDown) {
        {
            auto line = CreateConveyor(proto::Orientation::down);

            ConveyorInsert(proto::Orientation::up, line);
            EXPECT_EQ(line.lineSegment->left.lane.size(), 0);
            EXPECT_EQ(line.lineSegment->right.lane.size(), 0);
        }
        {
            auto line = CreateConveyor(proto::Orientation::down);

            ConveyorInsert(proto::Orientation::right, line);
            EXPECT_EQ(line.lineSegment->right.lane.size(), 1);
        }
        {
            auto line = CreateConveyor(proto::Orientation::down);

            ConveyorInsert(proto::Orientation::down, line);
            EXPECT_EQ(line.lineSegment->right.lane.size(), 1);
        }
        {
            auto line = CreateConveyor(proto::Orientation::down);

            ConveyorInsert(proto::Orientation::left, line);
            EXPECT_EQ(line.lineSegment->left.lane.size(), 1);
        }
    }

    TEST_F(ItemDropOffTest, InsertConveyorLeft) {
        {
            auto line = CreateConveyor(proto::Orientation::left);

            ConveyorInsert(proto::Orientation::up, line);
            EXPECT_EQ(line.lineSegment->left.lane.size(), 1);
        }
        {
            auto line = CreateConveyor(proto::Orientation::left);

            ConveyorInsert(proto::Orientation::right, line);
            EXPECT_EQ(line.lineSegment->left.lane.size(), 0);
            EXPECT_EQ(line.lineSegment->right.lane.size(), 0);
        }
        {
            auto line = CreateConveyor(proto::Orientation::left);

            ConveyorInsert(proto::Orientation::down, line);
            EXPECT_EQ(line.lineSegment->right.lane.size(), 1);
        }
        {
            auto line = CreateConveyor(proto::Orientation::left);

            ConveyorInsert(proto::Orientation::left, line);
            EXPECT_EQ(line.lineSegment->right.lane.size(), 1);
        }
    }

    TEST_F(ItemDropOffTest, DropoffConveyorNonStraight) {
        // -->
        // ^
        // | <--

        const auto right = CreateConveyor(proto::Orientation::right, ConveyorSegment::TerminationType::straight);
        const auto up    = CreateConveyor(proto::Orientation::up, ConveyorSegment::TerminationType::bend_right);
        auto left        = CreateConveyor(proto::Orientation::left, ConveyorSegment::TerminationType::bend_right);

        left.lineSegment->targetSegment = up.lineSegment.get();
        up.lineSegment->targetSegment   = right.lineSegment.get();

        left.lineSegmentIndex = 1;


        proto::Item item;

        ConveyorInsert(proto::Orientation::up, left);
        ASSERT_EQ(left.lineSegment->left.lane.size(), 1);
        EXPECT_DOUBLE_EQ(left.lineSegment->left.lane[0].dist.getAsDouble(), 0.5 + 0.7);
    }

    TEST_F(ItemDropOffTest, InsertAssemblyMachine) {
        data::PrototypeManager prototype_manager;

        auto recipe_pack = TestSetupRecipe(prototype_manager);

        proto::AssemblyMachineData asm_data;
        asm_data.ChangeRecipe(logicData_, prototype_manager, recipe_pack.recipe);

        asm_data.ingredientInv[0] = {recipe_pack.item1, 5, recipe_pack.item1};
        asm_data.ingredientInv[1] = {nullptr, 0, recipe_pack.item2};


        // Needs prototype data to register crafting callback
        proto::AssemblyMachine assembly_machine;
        targetProtoData_ = &assembly_machine;

        // Orientation doesn't matter
        EXPECT_TRUE(InsertAssemblyMachine({logicData_, {recipe_pack.item2, 10}, asm_data, proto::Orientation::up}));

        EXPECT_EQ(asm_data.ingredientInv[1].item, recipe_pack.item2);
        EXPECT_EQ(asm_data.ingredientInv[1].count, 9); // 1 used to begin crafting

        // Registered to start crafting
        EXPECT_NE(asm_data.deferralEntry.callbackIndex, 0);
    }

    TEST_F(ItemDropOffTest, InsertAssemblyMachineExceedStack) {
        // Inserters will not insert into assembly machines if it will exceed the current item's stack limit

        data::PrototypeManager prototype_manager;
        const auto recipe_pack = TestSetupRecipe(prototype_manager);

        proto::AssemblyMachineData asm_data;
        asm_data.ingredientInv.resize(2);
        asm_data.ingredientInv[0] = {recipe_pack.item1, 49, recipe_pack.item1};
        asm_data.ingredientInv[1] = {nullptr, 0, recipe_pack.item2};

        // 49 + 2 > 50
        EXPECT_FALSE(InsertAssemblyMachine({logicData_, {recipe_pack.item1, 2}, asm_data, proto::Orientation::up}));
    }

    TEST_F(ItemDropOffTest, CanInsertAssemblyMachine) {
        proto::AssemblyMachineData asm_data;

        proto::Item item;

        // No recipe
        EXPECT_FALSE(CanInsertAssemblyMachine({logicData_, {&item, 2}, asm_data, proto::Orientation::down}));


        data::PrototypeManager prototype_manager;

        auto recipe_pack = TestSetupRecipe(prototype_manager);
        asm_data.ChangeRecipe(logicData_, prototype_manager, recipe_pack.recipe);

        // Has recipe, wrong item
        EXPECT_FALSE(CanInsertAssemblyMachine({logicData_, {&item, 2}, asm_data, proto::Orientation::down}));

        // Has recipe, correct item
        DropOffParams args{logicData_, {recipe_pack.item1, 2000}, asm_data, proto::Orientation::down};
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
        explicit InserterPickupTest() : InserterPickup(proto::Orientation::up) {}

        void SetUp() override {
            worldData_.EmplaceChunk(0, 0);
        }

    protected:
        WorldData worldData_;
        LogicData logicData_;

        proto::Inserter inserterProto_;

        /// Item which will be on conveyor segments from CreateConveyor
        proto::Item lineItem_;

        /// Creates a conveyor with  1 item on each side
        proto::ConveyorData CreateConveyor(const proto::Orientation orientation) const {

            const auto segment =
                std::make_shared<ConveyorSegment>(orientation, ConveyorSegment::TerminationType::straight, 2);

            segment->InsertItem(false, 0.5, lineItem_);
            segment->InsertItem(true, 0.5, lineItem_);

            return proto::ConveyorData{segment};
        }


        /// \param orientation Orientation to line being picked up from
        void PickupLine(const proto::Orientation orientation, proto::ConveyorData& line_data) {
            constexpr int pickup_amount = 1;

            const auto result = PickupTransportBelt(
                {logicData_, 1, proto::RotationDegreeT(kMaxInserterDegree), pickup_amount, line_data, orientation});

            EXPECT_TRUE(result.first);

            EXPECT_NE(result.second.item, nullptr);
            EXPECT_EQ(result.second.count, pickup_amount);
        }
    };

    TEST_F(InserterPickupTest, GetPickupFunc) {
        worldData_.GetTile(2, 4)->GetLayer(TileLayer::entity).MakeUniqueData<proto::ContainerEntityData>(1);

        auto set_prototype = [&](proto::Entity& entity_proto) {
            auto& layer         = worldData_.GetTile(2, 4)->GetLayer(TileLayer::entity);
            layer.prototypeData = &entity_proto;
        };


        // No: Empty tile cannot be picked up from
        EXPECT_FALSE(this->Initialize(worldData_, {2, 4}));
        EXPECT_FALSE(this->IsInitialized());


        // Ok: Transport belt can be picked up from
        proto::TransportBelt belt;
        set_prototype(belt);

        EXPECT_TRUE(this->Initialize(worldData_, {2, 4}));
        EXPECT_TRUE(this->IsInitialized());


        // No: Mining drill
        proto::MiningDrill drill;
        set_prototype(drill);

        EXPECT_FALSE(this->Initialize(worldData_, {2, 4}));
        EXPECT_TRUE(this->IsInitialized()); // Still initialized from conveyor belt


        // Ok: Container
        proto::ContainerEntity container;
        set_prototype(container);

        EXPECT_TRUE(this->Initialize(worldData_, {2, 4}));
        EXPECT_TRUE(this->IsInitialized());


        // Ok: Assembly machine
        proto::AssemblyMachine assembly_machine;
        TestSetupAssemblyMachine(worldData_, {3, 4}, assembly_machine);

        EXPECT_TRUE(this->Initialize(worldData_, {4, 5}));
        EXPECT_TRUE(this->IsInitialized());
        EXPECT_EQ(this->targetProtoData_, &assembly_machine);
    }

    TEST_F(InserterPickupTest, PickupContainerEntity) {
        proto::ContainerEntity container_entity;
        auto& container_layer = TestSetupContainer(worldData_, {2, 4}, container_entity);
        auto& container_data  = *container_layer.GetUniqueData();

        proto::Item item;
        auto& inv = container_layer.GetUniqueData<proto::ContainerEntityData>()->inventory;
        inv[0]    = {&item, 10};


        PickupContainerEntity({logicData_, 1, proto::RotationDegreeT(179), 1, container_data, proto::Orientation::up});
        EXPECT_EQ(inv[0].count, 10); // No items picked up, not 180 degrees


        PickupParams args = {logicData_, 1, proto::RotationDegreeT(180), 2, container_data, proto::Orientation::up};

        EXPECT_EQ(GetPickupContainerEntity(args), &item);

        auto result = PickupContainerEntity(args);
        EXPECT_EQ(inv[0].count, 8); // At 180 degrees, 2 items picked up
        EXPECT_EQ(result.second.item, &item);
        EXPECT_EQ(result.second.count, 2);
    }

    TEST_F(InserterPickupTest, PickupConveyorUp) {
        // Line is above inserter
        {
            auto line = CreateConveyor(proto::Orientation::up);

            PickupLine(proto::Orientation::up, line);
            EXPECT_EQ(line.lineSegment->right.lane.size(), 0);
        }
        {
            auto line = CreateConveyor(proto::Orientation::up);

            PickupLine(proto::Orientation::right, line);
            EXPECT_EQ(line.lineSegment->right.lane.size(), 0);
        }
        {
            auto line = CreateConveyor(proto::Orientation::up);

            PickupLine(proto::Orientation::down, line);
            EXPECT_EQ(line.lineSegment->left.lane.size(), 0);
        }
        {
            auto line = CreateConveyor(proto::Orientation::up);

            PickupLine(proto::Orientation::left, line);
            EXPECT_EQ(line.lineSegment->left.lane.size(), 0);
        }
    }

    TEST_F(InserterPickupTest, PickupConveyorRight) {
        {
            auto line = CreateConveyor(proto::Orientation::right);

            PickupLine(proto::Orientation::up, line);
            EXPECT_EQ(line.lineSegment->left.lane.size(), 0);
        }
        {
            auto line = CreateConveyor(proto::Orientation::right);

            PickupLine(proto::Orientation::right, line);
            EXPECT_EQ(line.lineSegment->right.lane.size(), 0);
        }
        {
            auto line = CreateConveyor(proto::Orientation::right);

            PickupLine(proto::Orientation::down, line);
            EXPECT_EQ(line.lineSegment->right.lane.size(), 0);
        }
        {
            auto line = CreateConveyor(proto::Orientation::right);

            PickupLine(proto::Orientation::left, line);
            EXPECT_EQ(line.lineSegment->left.lane.size(), 0);
        }
    }

    TEST_F(InserterPickupTest, PickupConveyorDown) {
        {
            auto line = CreateConveyor(proto::Orientation::down);

            PickupLine(proto::Orientation::up, line);
            EXPECT_EQ(line.lineSegment->left.lane.size(), 0);
        }
        {
            auto line = CreateConveyor(proto::Orientation::down);

            PickupLine(proto::Orientation::right, line);
            EXPECT_EQ(line.lineSegment->left.lane.size(), 0);
        }
        {
            auto line = CreateConveyor(proto::Orientation::down);

            PickupLine(proto::Orientation::down, line);
            EXPECT_EQ(line.lineSegment->right.lane.size(), 0);
        }
        {
            auto line = CreateConveyor(proto::Orientation::down);

            PickupLine(proto::Orientation::left, line);
            EXPECT_EQ(line.lineSegment->right.lane.size(), 0);
        }
    }

    TEST_F(InserterPickupTest, PickupConveyorLeft) {
        {
            auto line = CreateConveyor(proto::Orientation::left);

            PickupLine(proto::Orientation::up, line);
            EXPECT_EQ(line.lineSegment->right.lane.size(), 0);
        }
        {
            auto line = CreateConveyor(proto::Orientation::left);

            PickupLine(proto::Orientation::right, line);
            EXPECT_EQ(line.lineSegment->left.lane.size(), 0);
        }
        {
            auto line = CreateConveyor(proto::Orientation::left);

            PickupLine(proto::Orientation::down, line);
            EXPECT_EQ(line.lineSegment->left.lane.size(), 0);
        }
        {
            auto line = CreateConveyor(proto::Orientation::left);

            PickupLine(proto::Orientation::left, line);
            EXPECT_EQ(line.lineSegment->right.lane.size(), 0);
        }
    }

    TEST_F(InserterPickupTest, PickupConveyorNonStraight) {
        // -->
        // ^
        // | <--

        const auto right =
            std::make_shared<ConveyorSegment>(proto::Orientation::right, ConveyorSegment::TerminationType::straight, 2);

        const auto up =
            std::make_shared<ConveyorSegment>(proto::Orientation::up, ConveyorSegment::TerminationType::bend_right, 2);

        const auto left = std::make_shared<ConveyorSegment>(
            proto::Orientation::left, ConveyorSegment::TerminationType::bend_right, 2);

        left->targetSegment = up.get();
        up->targetSegment   = right.get();

        proto::ConveyorData line{left};
        line.lineSegmentIndex = 1;


        const proto::Item item;

        left->AppendItem(true, 0.5 + 0.7, item);
        PickupLine(proto::Orientation::up, line);
        EXPECT_EQ(left->left.lane.size(), 0);

        left->AppendItem(false, 0.5 + 0.3, item);
        PickupLine(proto::Orientation::up, line);
        EXPECT_EQ(left->right.lane.size(), 0);
    }

    TEST_F(InserterPickupTest, PickupConveyorAlternativeSide) {
        // If the preferred lane is available, inserter will attempt to pick up from other lane
        auto line = CreateConveyor(proto::Orientation::up);

        EXPECT_EQ(GetPickupTransportBelt({logicData_, 1, proto::RotationDegreeT(180), 1, line, proto::Orientation::up}),
                  &lineItem_);

        // Should set index to 0 since a item was removed
        line.lineSegment->right.index = 10;

        PickupLine(proto::Orientation::up, line);
        EXPECT_EQ(line.lineSegment->right.lane.size(), 0);
        EXPECT_EQ(line.lineSegment->right.index, 0);


        // Use alternative side
        EXPECT_EQ(GetPickupTransportBelt({logicData_, 1, proto::RotationDegreeT(180), 1, line, proto::Orientation::up}),
                  &lineItem_);

        line.lineSegment->left.index = 10;

        PickupLine(proto::Orientation::up, line);
        EXPECT_EQ(line.lineSegment->left.lane.size(), 0);
        EXPECT_EQ(line.lineSegment->left.index, 0);
    }


    TEST_F(InserterPickupTest, PickupAssemblyMachine) {
        data::PrototypeManager prototype_manager;

        proto::AssemblyMachine asm_machine;
        auto& layer = TestSetupAssemblyMachine(worldData_, {0, 0}, asm_machine);
        auto* data  = layer.GetUniqueData<proto::AssemblyMachineData>();

        // Does nothing as there is no recipe yet
        PickupAssemblyMachine(
            {logicData_, 2, proto::RotationDegreeT(kMaxInserterDegree), 2, *data, proto::Orientation::up});

        // ======================================================================

        auto recipe_pack = TestSetupRecipe(prototype_manager);


        data->ChangeRecipe(logicData_, prototype_manager, recipe_pack.recipe);

        // No items in product inventory
        EXPECT_FALSE(PickupAssemblyMachine(
                         {logicData_, 2, proto::RotationDegreeT(kMaxInserterDegree), 2, *data, proto::Orientation::up})
                         .first);


        // Has items in product inventory
        data->ingredientInv[0] = {recipe_pack.item1, 1};
        data->ingredientInv[1] = {recipe_pack.item2, 1};

        data->productInv[0] = {recipe_pack.itemProduct, 10, recipe_pack.itemProduct};

        targetProtoData_ = &asm_machine;

        PickupParams args{logicData_, 2, proto::RotationDegreeT(kMaxInserterDegree), 2, *data, proto::Orientation::up};

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
