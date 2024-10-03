from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, OpaqueFunction
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import ComposableNodeContainer, LoadComposableNodes, Node
from launch_ros.descriptions import ComposableNode

def launch_setup(context, *args, **kwargs):
    camera_configs = [
        {"name": "hdr_left", "namespace": "/gt_box/", "video_device": "/dev/video2", "frame_id": "hdr_left"},
        #{"name": "hdr_front", "namespace": "/gt_box/", "video_device": "/dev/video3", "frame_id": "hdr_front"},
        #{"name": "hdr_right", "namespace": "/gt_box/", "video_device": "/dev/video4", "frame_id": "hdr_right"},
    ]

    composable_nodes = []

    for config in camera_configs:
        composable_nodes.append(
            ComposableNode(
                package="v4l2_camera",
                plugin="v4l2_camera::V4L2Camera",
                name=f'v4l2_camera_{config["name"]}',
                namespace=config["namespace"],
                remappings=[
                    ("image_raw", f"{config['name']}/image_raw"),
                    ("image_raw/compressed", f"{config['name']}/image_raw/compressed"),
                    ("image_raw/compressedDepth", f"{config['name']}/image_raw/compressedDepth"),
                    ("image_raw/theora", f"{config['name']}/image_raw/theora"),
                    ("camera_info", f"{config['name']}/camera_info"),
                ],
                parameters=[{
                    "video_device": config["video_device"],
                    "frame_id": config["frame_id"],
                    "image_size": (1920, 1080),
                }],
                extra_arguments=[{"use_intra_process_comms": LaunchConfiguration("use_intra_process")}],
            )
        )

    if LaunchConfiguration('container').perform(context) == '':
        # Start a new container if not provided
        return [
            ComposableNodeContainer(
                name='v4l2_camera_container',
                namespace='',
                package='rclcpp_components',
                executable='component_container_mt',
                composable_node_descriptions=composable_nodes,
                output='screen',
            )
        ]
    else:
        # Load into an existing container
        return [
            LoadComposableNodes(
                target_container=LaunchConfiguration('container'),
                composable_node_descriptions=composable_nodes,
            )
        ]


    
def generate_launch_description():
   launch_arguments = [
       DeclareLaunchArgument(
           'container', default_value='',
           description='Container name to load composable nodes into it. If not specified, a new container will be created.'
       ),
       DeclareLaunchArgument(
           'use_intra_process', default_value='False',
           description='Flag to use ROS2 intra-process communication'
       ),
   ]
   return LaunchDescription(
       [
           *launch_arguments,
           OpaqueFunction(function=launch_setup),
       ]
   )

# def generate_launch_description():
#     camera_configs = [
#         {"name": "hdr_left", "namespace": "/gt_box/", "video_device": "/dev/video2", "frame_id": "hdr_left"},
#         #{"name": "hdr_front", "namespace": "/gt_box/", "video_device": "/dev/video3", "frame_id": "hdr_front"},
#         #{"name": "hdr_right", "namespace": "/gt_box/", "video_device": "/dev/video4", "frame_id": "hdr_right"},
#     ]

#     nodes = []

#     for config in camera_configs:
#         nodes.append(
#             Node(
#                 package="v4l2_camera",
#                 plugin="v4l2_camera::V4L2Camera",
#                 name=f'v4l2_camera_{config["name"]}',
#                 namespace=config["namespace"],
#                 remappings=[
#                     ("image_raw", f"{config['name']}/image_raw"),
#                     ("image_raw/compressed", f"{config['name']}/image_raw/compressed"),
#                     ("image_raw/compressedDepth", f"{config['name']}/image_raw/compressedDepth"),
#                     ("image_raw/theora", f"{config['name']}/image_raw/theora"),
#                     ("camera_info", f"{config['name']}/camera_info"),
#                 ],
#                 parameters=[{"frame_id":"a",}],
#                 #parameters=[{
#                 #    "video_device": config["video_device"],
#                 #    "frame_id": config["frame_id"],
#                 #    "image_size": [1920, 1080],
#                 #}],
#             )
#         )
#         
#     return LaunchDescription( nodes )