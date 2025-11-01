#!/bin/bash
# SenseSimulation 仿真环境

GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

echo -e "${GREEN}🌍 SenseSimulation 仿真环境${NC}"

run() {
    echo -e "${GREEN}🌍 启动仿真...${NC}"
    
    if [ ! -f "install/setup.bash" ]; then
        echo -e "${RED}❌ 请先编译: sh toBuild.sh${NC}"
        return 1
    fi
    
    source install/setup.bash
    ros2 launch simulation_bringup simulation.launch.py
}

build() {
    echo -e "${GREEN}🔨 编译...${NC}"
    sh toBuild.sh
}

echo -e "${YELLOW}💡 输入 ${GREEN}run${YELLOW} 启动仿真环境${NC}\n"
