#include <moveit/robot_model_loader/robot_model_loader.hpp>
#include <moveit/robot_model/robot_model.hpp>
#include <moveit/robot_state/robot_state.hpp>

void printJointValues(const moveit::core::RobotModelPtr &kinematic_model, moveit::core::RobotStatePtr robot_state, const rclcpp::Logger& LOGGER);


int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    rclcpp::NodeOptions node_options;
    // This enables loading undeclared parameters
    // best practice would be to declare parameters in the corresponding classes
    // and provide descriptions about expected use
    node_options.automatically_declare_parameters_from_overrides(true);
    auto node = rclcpp::Node::make_shared("robot_model_and_state_tutorial", node_options);
    const auto &LOGGER = node->get_logger();

    // Start
    // ^^^^^
    // Setting up to start using the RobotModel class is very easy. In
    // general, you will find that most higher-level components will
    // return a shared pointer to the RobotModel. You should always use
    // that when possible. In this example, we will start with such a
    // shared pointer and discuss only the basic API. You can have a
    // look at the actual code API for these classes to get more
    // information about how to use more features provided by these
    // classes.
    //
    // We will start by instantiating a
    // `RobotModelLoader`_
    // object, which will look up
    // the robot description on the ROS parameter server and construct a
    // :moveit_codedir:`RobotModel<moveit_core/robot_model/include/moveit/robot_model/robot_model.hpp>` for us to use.
    //
    // .. _RobotModelLoader:
    //     https://github.com/moveit/moveit2/blob/main/moveit_ros/planning/robot_model_loader/include/moveit/robot_model_loader/robot_model_loader.hpp
    robot_model_loader::RobotModelLoader robot_model_loader(node);
    const moveit::core::RobotModelPtr &kinematic_model = robot_model_loader.getModel();
    RCLCPP_INFO(LOGGER, "Model frame: %s", kinematic_model->getModelFrame().c_str());

    // Using the :moveit_codedir:`RobotModel<moveit_core/robot_model/include/moveit/robot_model/robot_model.hpp>`, we can
    // construct a :moveit_codedir:`RobotState<moveit_core/robot_state/include/moveit/robot_state/robot_state.hpp>` that
    // maintains the configuration of the robot. We will set all joints in the state to their default values. We can then
    // get a :moveit_codedir:`JointModelGroup<moveit_core/robot_model/include/moveit/robot_model/joint_model_group.hpp>`,
    // which represents the robot model for a particular group, e.g. the "panda_arm" of the Panda robot.
    moveit::core::RobotStatePtr robot_state(new moveit::core::RobotState(kinematic_model));
    robot_state->setToDefaultValues();
    const moveit::core::JointModelGroup *joint_model_group = kinematic_model->getJointModelGroup("xarm6");

    const std::vector<std::string> &joint_names = joint_model_group->getVariableNames();

    // Get Joint Values
    // ^^^^^^^^^^^^^^^^
    // We can retrieve the current set of joint values stored in the state for the Panda arm.
    std::vector<double> joint_values;
    robot_state->copyJointGroupPositions(joint_model_group, joint_values);
    for (std::size_t i = 0; i < joint_names.size(); ++i)
    {
        RCLCPP_INFO(LOGGER, "Joint %s: %f", joint_names[i].c_str(), joint_values[i]);
    }

    // // Joint Limits
    // // ^^^^^^^^^^^^
    // // setJointGroupPositions() does not enforce joint limits by itself, but a call to enforceBounds() will do it.
    // /* Set one joint in the Panda arm outside its joint limit */
    // joint_values[0] = 5.57;
    // robot_state->setJointGroupPositions(joint_model_group, joint_values);

    // /* Check whether any joint is outside its joint limits */
    // RCLCPP_INFO_STREAM(LOGGER, "Current state is " << (robot_state->satisfiesBounds() ? "valid" : "not valid"));

    // /* Enforce the joint limits for this state and check again*/
    // robot_state->enforceBounds();
    // RCLCPP_INFO_STREAM(LOGGER, "Current state is " << (robot_state->satisfiesBounds() ? "valid" : "not valid"));

    // ========================================================================
    // checkpoint
    // ========================================================================

    // // Forward Kinematics
    // // ^^^^^^^^^^^^^^^^^^
    // // Now, we can compute forward kinematics for a set of random joint
    // // values. Note that we would like to find the pose of the
    // // "link6" which is the most distal link in the
    // // "xarm6" group of the robot.
    // robot_state->setToRandomPositions(joint_model_group);
    const Eigen::Isometry3d &end_effector_state = robot_state->getGlobalLinkTransform("link6");

    // /* Print end-effector pose. Remember that this is in the model frame */
    // RCLCPP_INFO_STREAM(LOGGER, "Translation: \n"
    //                                << end_effector_state.translation() << "\n");
    // RCLCPP_INFO_STREAM(LOGGER, "Rotation: \n"
    //                                << end_effector_state.rotation() << "\n");

    // Inverse Kinematics
    // ^^^^^^^^^^^^^^^^^^
    // We can now solve inverse kinematics (IK) for the Panda robot.
    // To solve IK, we will need the following:
    //
    //  * The desired pose of the end-effector (by default, this is the last link in the "panda_arm" chain):
    //    end_effector_state that we computed in the step above.
    //  * The timeout: 0.1 s
    double timeout = 0.1;
    bool found_ik = robot_state->setFromIK(joint_model_group, end_effector_state, timeout);

    // Now, we can print out the IK solution (if found):
    if (found_ik)
    {
        robot_state->copyJointGroupPositions(joint_model_group, joint_values);
        for (std::size_t i = 0; i < joint_names.size(); ++i)
        {
            RCLCPP_INFO(LOGGER, "Joint %s: %f", joint_names[i].c_str(), joint_values[i]);
        }
    }
    else
    {
        RCLCPP_INFO(LOGGER, "Did not find IK solution");
    }

    // Get the Jacobian
    // ^^^^^^^^^^^^^^^^
    // We can also get the Jacobian from the
    // :moveit_codedir:`RobotState<moveit_core/robot_state/include/moveit/robot_state/robot_state.hpp>`.
    Eigen::Vector3d reference_point_position(0.0, 0.0, 0.0);
    Eigen::MatrixXd jacobian;
    robot_state->getJacobian(joint_model_group, robot_state->getLinkModel(joint_model_group->getLinkModelNames().back()),
                             reference_point_position, jacobian);
    RCLCPP_INFO_STREAM(LOGGER, "Jacobian: \n"
                                   << jacobian << "\n");



    rclcpp::shutdown();
    return 0;
}



void printJointValues(const moveit::core::RobotModelPtr &kinematic_model, moveit::core::RobotStatePtr robot_state, const rclcpp::Logger&  LOGGER){
    RCLCPP_INFO(LOGGER, "printJointValues()");

    const moveit::core::JointModelGroup *joint_model_group = kinematic_model->getJointModelGroup("xarm6");

    const std::vector<std::string> &joint_names = joint_model_group->getVariableNames();

    std::vector<double> joint_values;

    robot_state->copyJointGroupPositions(joint_model_group, joint_values);
    
    for (std::size_t i = 0; i < joint_names.size(); ++i)
    {
        RCLCPP_INFO(LOGGER, "Joint %s: %f", joint_names[i].c_str(), joint_values[i]);
    }

}