#!/bin/bash

################################################################################
# 基础环境一键部署脚本
# 作者: AI Assistant
# 功能: 自动安装和配置常用开发环境
# 支持: Ubuntu/Debian/CentOS/Fedora/Arch Linux
# 使用: curl -fsSL https://github.com/Aununo/Programs/blob/main/Utils/setup_env.sh | sudo bash
################################################################################

set -e  # 遇到错误立即退出

# ============================================================================
# 颜色定义
# ============================================================================
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
MAGENTA='\033[0;35m'
CYAN='\033[0;36m'
WHITE='\033[1;37m'
NC='\033[0m' # No Color
BOLD='\033[1m'

# ============================================================================
# 全局变量
# ============================================================================
LOG_FILE="/var/log/setup_env_$(date +%Y%m%d_%H%M%S).log"
BACKUP_DIR="/var/backup/setup_env_$(date +%Y%m%d_%H%M%S)"
DISTRO=""
PKG_MANAGER=""
RETRY_TIMES=3
INTERACTIVE_MODE=true

# 要安装的组件（默认全部）
INSTALL_GIT=true
INSTALL_DOCKER=true
INSTALL_PYTHON=true
INSTALL_CURL=true
CONFIG_SSH=true
CHANGE_MIRROR=false
UPDATE_SYSTEM=true

# ============================================================================
# 日志函数
# ============================================================================
log() {
    echo -e "[$(date '+%Y-%m-%d %H:%M:%S')] $*" | tee -a "$LOG_FILE"
}

log_info() {
    echo -e "${BLUE}[INFO]${NC} $*" | tee -a "$LOG_FILE"
}

log_success() {
    echo -e "${GREEN}[✓]${NC} $*" | tee -a "$LOG_FILE"
}

log_warning() {
    echo -e "${YELLOW}[⚠]${NC} $*" | tee -a "$LOG_FILE"
}

log_error() {
    echo -e "${RED}[✗]${NC} $*" | tee -a "$LOG_FILE"
}

log_step() {
    echo -e "${CYAN}${BOLD}[STEP]${NC} $*" | tee -a "$LOG_FILE"
}

# ============================================================================
# 打印带边框的标题
# ============================================================================
print_banner() {
    local text="$1"
    local width=80
    echo -e "${MAGENTA}"
    echo "╔$(printf '═%.0s' $(seq 1 $((width-2))))╗"
    printf "║ %-$((width-4))s ║\n" "$text"
    echo "╚$(printf '═%.0s' $(seq 1 $((width-2))))╝"
    echo -e "${NC}"
}

# ============================================================================
# 进度条
# ============================================================================
progress_bar() {
    local current=$1
    local total=$2
    local width=50
    local percentage=$((current * 100 / total))
    local completed=$((width * current / total))
    local remaining=$((width - completed))
    
    printf "\r${CYAN}进度: [${GREEN}"
    printf "%${completed}s" | tr ' ' '█'
    printf "${NC}%${remaining}s${CYAN}]${NC} ${BOLD}%3d%%${NC}" " " "$percentage"
}

# ============================================================================
# 检测Linux发行版
# ============================================================================
detect_distro() {
    log_step "检测Linux发行版..."
    
    if [ -f /etc/os-release ]; then
        . /etc/os-release
        DISTRO=$ID
        
        case $DISTRO in
            ubuntu|debian)
                PKG_MANAGER="apt"
                log_success "检测到系统: $PRETTY_NAME (使用 apt)"
                ;;
            centos|rhel|fedora)
                PKG_MANAGER="dnf"
                if ! command -v dnf &> /dev/null; then
                    PKG_MANAGER="yum"
                fi
                log_success "检测到系统: $PRETTY_NAME (使用 $PKG_MANAGER)"
                ;;
            arch|manjaro)
                PKG_MANAGER="pacman"
                log_success "检测到系统: $PRETTY_NAME (使用 pacman)"
                ;;
            *)
                log_warning "未知发行版: $DISTRO，将尝试使用 apt"
                PKG_MANAGER="apt"
                ;;
        esac
    else
        log_error "无法检测系统版本"
        exit 1
    fi
    
    log_info "系统架构: $(uname -m)"
    log_info "内核版本: $(uname -r)"
}

# ============================================================================
# 检查root权限
# ============================================================================
check_root() {
    if [ "$EUID" -ne 0 ]; then 
        log_error "请使用 sudo 运行此脚本"
        exit 1
    fi
}

# ============================================================================
# 创建备份目录
# ============================================================================
create_backup_dir() {
    mkdir -p "$BACKUP_DIR"
    log_info "备份目录: $BACKUP_DIR"
}

# ============================================================================
# 带重试的执行命令
# ============================================================================
retry_command() {
    local cmd="$*"
    local count=0
    
    while [ $count -lt $RETRY_TIMES ]; do
        if eval "$cmd"; then
            return 0
        else
            count=$((count + 1))
            if [ $count -lt $RETRY_TIMES ]; then
                log_warning "命令执行失败，重试 $count/$RETRY_TIMES..."
                sleep 2
            fi
        fi
    done
    
    log_error "命令执行失败，已重试 $RETRY_TIMES 次: $cmd"
    return 1
}

# ============================================================================
# 切换国内镜像源
# ============================================================================
change_mirror() {
    log_step "切换到国内镜像源..."
    
    case $PKG_MANAGER in
        apt)
            # 备份原始sources.list
            if [ -f /etc/apt/sources.list ]; then
                cp /etc/apt/sources.list "$BACKUP_DIR/sources.list.backup"
                log_info "已备份 /etc/apt/sources.list"
            fi
            
            # 检测Ubuntu版本
            local codename=$(lsb_release -cs 2>/dev/null || echo "focal")
            
            # 使用阿里云镜像
            cat > /etc/apt/sources.list << EOF
# 阿里云镜像源
deb https://mirrors.aliyun.com/ubuntu/ $codename main restricted universe multiverse
deb https://mirrors.aliyun.com/ubuntu/ $codename-security main restricted universe multiverse
deb https://mirrors.aliyun.com/ubuntu/ $codename-updates main restricted universe multiverse
deb https://mirrors.aliyun.com/ubuntu/ $codename-backports main restricted universe multiverse
EOF
            log_success "已切换到阿里云镜像源"
            ;;
            
        yum|dnf)
            # 备份原始repo
            if [ -d /etc/yum.repos.d ]; then
                cp -r /etc/yum.repos.d "$BACKUP_DIR/yum.repos.d.backup"
            fi
            
            # CentOS/Fedora切换到阿里云
            if [ "$DISTRO" = "centos" ]; then
                sed -e 's|^mirrorlist=|#mirrorlist=|g' \
                    -e 's|^#baseurl=http://mirror.centos.org|baseurl=https://mirrors.aliyun.com|g' \
                    -i.bak /etc/yum.repos.d/CentOS-*.repo
            fi
            log_success "已切换到阿里云镜像源"
            ;;
            
        pacman)
            # 备份pacman配置
            cp /etc/pacman.d/mirrorlist "$BACKUP_DIR/mirrorlist.backup"
            
            # 添加清华源到最前面
            sed -i '1i Server = https://mirrors.tuna.tsinghua.edu.cn/archlinux/$repo/os/$arch' /etc/pacman.d/mirrorlist
            log_success "已添加清华镜像源"
            ;;
    esac
}

# ============================================================================
# 更新系统
# ============================================================================
update_system() {
    log_step "更新系统软件包..."
    
    case $PKG_MANAGER in
        apt)
            retry_command "apt update -y"
            ;;
        yum)
            retry_command "yum update -y"
            ;;
        dnf)
            retry_command "dnf update -y"
            ;;
        pacman)
            retry_command "pacman -Sy --noconfirm"
            ;;
    esac
    
    log_success "系统软件包列表更新完成"
}

# ============================================================================
# 安装软件包
# ============================================================================
install_package() {
    local package=$1
    
    case $PKG_MANAGER in
        apt)
            retry_command "apt install -y $package"
            ;;
        yum)
            retry_command "yum install -y $package"
            ;;
        dnf)
            retry_command "dnf install -y $package"
            ;;
        pacman)
            retry_command "pacman -S --noconfirm $package"
            ;;
    esac
}

# ============================================================================
# 安装Git
# ============================================================================
install_git() {
    log_step "安装 Git..."
    
    if command -v git &> /dev/null; then
        log_success "Git 已安装 - 版本: $(git --version)"
    else
        install_package git
        log_success "Git 安装完成 - 版本: $(git --version)"
    fi
}

# ============================================================================
# 安装Docker
# ============================================================================
install_docker() {
    log_step "安装 Docker..."
    
    if command -v docker &> /dev/null; then
        log_success "Docker 已安装 - 版本: $(docker --version)"
        return
    fi
    
    case $PKG_MANAGER in
        apt)
            # 安装依赖
            install_package "ca-certificates curl gnupg lsb-release"
            
            # 添加Docker官方GPG密钥
            install -m 0755 -d /etc/apt/keyrings
            
            # 根据是否使用国内镜像选择不同的下载源
            if [ "$CHANGE_MIRROR" = true ]; then
                curl -fsSL https://mirrors.aliyun.com/docker-ce/linux/ubuntu/gpg | gpg --dearmor -o /etc/apt/keyrings/docker.gpg
            else
                curl -fsSL https://download.docker.com/linux/ubuntu/gpg | gpg --dearmor -o /etc/apt/keyrings/docker.gpg
            fi
            chmod a+r /etc/apt/keyrings/docker.gpg
            
            # 添加Docker仓库
            if [ "$CHANGE_MIRROR" = true ]; then
                echo \
                  "deb [arch=$(dpkg --print-architecture) signed-by=/etc/apt/keyrings/docker.gpg] https://mirrors.aliyun.com/docker-ce/linux/ubuntu \
                  $(lsb_release -cs) stable" | tee /etc/apt/sources.list.d/docker.list > /dev/null
            else
                echo \
                  "deb [arch=$(dpkg --print-architecture) signed-by=/etc/apt/keyrings/docker.gpg] https://download.docker.com/linux/ubuntu \
                  $(lsb_release -cs) stable" | tee /etc/apt/sources.list.d/docker.list > /dev/null
            fi
            
            apt update -y
            install_package "docker-ce docker-ce-cli containerd.io docker-buildx-plugin docker-compose-plugin"
            ;;
            
        yum|dnf)
            install_package "yum-utils"
            $PKG_MANAGER config-manager --add-repo https://download.docker.com/linux/centos/docker-ce.repo
            install_package "docker-ce docker-ce-cli containerd.io docker-buildx-plugin docker-compose-plugin"
            ;;
            
        pacman)
            install_package "docker docker-compose"
            ;;
    esac
    
    # 启动Docker服务
    systemctl enable docker
    systemctl start docker
    
    log_success "Docker 安装完成 - 版本: $(docker --version)"
}

# ============================================================================
# 安装Python
# ============================================================================
install_python() {
    log_step "安装 Python3 和 pip3..."
    
    if ! command -v python3 &> /dev/null; then
        case $PKG_MANAGER in
            apt)
                install_package "python3 python3-pip"
                ;;
            yum|dnf)
                install_package "python3 python3-pip"
                ;;
            pacman)
                install_package "python python-pip"
                ;;
        esac
    fi
    
    log_success "Python3 - 版本: $(python3 --version)"
    log_success "pip3 - 版本: $(pip3 --version 2>/dev/null || echo 'N/A')"
}

# ============================================================================
# 安装curl
# ============================================================================
install_curl() {
    log_step "安装 curl..."
    
    if command -v curl &> /dev/null; then
        log_success "curl 已安装 - 版本: $(curl --version | head -n 1)"
    else
        install_package curl
        log_success "curl 安装完成 - 版本: $(curl --version | head -n 1)"
    fi
}

# ============================================================================
# 配置SSH
# ============================================================================
configure_ssh() {
    log_step "配置 SSH..."
    
    # 安装SSH服务
    if ! command -v sshd &> /dev/null; then
        case $PKG_MANAGER in
            apt)
                install_package openssh-server
                ;;
            yum|dnf)
                install_package openssh-server
                ;;
            pacman)
                install_package openssh
                ;;
        esac
    fi
    
    # 备份SSH配置
    if [ -f /etc/ssh/sshd_config ]; then
        cp /etc/ssh/sshd_config "$BACKUP_DIR/sshd_config.backup"
        log_info "已备份 SSH 配置文件"
    fi
    
    # 禁用密码登录
    sed -i 's/#PasswordAuthentication yes/PasswordAuthentication no/' /etc/ssh/sshd_config
    sed -i 's/^PasswordAuthentication yes/PasswordAuthentication no/' /etc/ssh/sshd_config
    
    # 设置SSH超时时间为10分钟
    sed -i 's/#ClientAliveInterval 0/ClientAliveInterval 120/' /etc/ssh/sshd_config
    sed -i 's/^ClientAliveInterval.*/ClientAliveInterval 120/' /etc/ssh/sshd_config
    sed -i 's/#ClientAliveCountMax 3/ClientAliveCountMax 5/' /etc/ssh/sshd_config
    sed -i 's/^ClientAliveCountMax.*/ClientAliveCountMax 5/' /etc/ssh/sshd_config
    
    # 重启SSH服务
    systemctl restart sshd 2>/dev/null || systemctl restart ssh 2>/dev/null
    
    log_success "SSH 配置完成"
    log_info "  - 密码登录已禁用"
    log_info "  - SSH 超时时间: 10分钟 (120秒心跳 × 5次)"
}

# ============================================================================
# 系统升级
# ============================================================================
upgrade_system() {
    log_step "升级所有软件包..."
    
    case $PKG_MANAGER in
        apt)
            apt upgrade -y
            apt autoremove -y
            apt autoclean -y
            ;;
        yum)
            yum upgrade -y
            yum autoremove -y
            ;;
        dnf)
            dnf upgrade -y
            dnf autoremove -y
            ;;
        pacman)
            pacman -Syu --noconfirm
            pacman -Sc --noconfirm
            ;;
    esac
    
    log_success "系统升级完成"
}

# ============================================================================
# 交互式菜单
# ============================================================================
interactive_menu() {
    print_banner "基础环境一键部署脚本 - 交互式安装"
    
    echo -e "${CYAN}请选择要安装的组件:${NC}"
    echo ""
    
    read -p "$(echo -e ${YELLOW}是否安装 Git? [Y/n]: ${NC})" choice
    [[ "$choice" =~ ^[Nn]$ ]] && INSTALL_GIT=false
    
    read -p "$(echo -e ${YELLOW}是否安装 Docker? [Y/n]: ${NC})" choice
    [[ "$choice" =~ ^[Nn]$ ]] && INSTALL_DOCKER=false
    
    read -p "$(echo -e ${YELLOW}是否安装 Python3? [Y/n]: ${NC})" choice
    [[ "$choice" =~ ^[Nn]$ ]] && INSTALL_PYTHON=false
    
    read -p "$(echo -e ${YELLOW}是否安装 curl? [Y/n]: ${NC})" choice
    [[ "$choice" =~ ^[Nn]$ ]] && INSTALL_CURL=false
    
    read -p "$(echo -e ${YELLOW}是否配置 SSH? [Y/n]: ${NC})" choice
    [[ "$choice" =~ ^[Nn]$ ]] && CONFIG_SSH=false
    
    read -p "$(echo -e ${YELLOW}是否切换到国内镜像源? [y/N]: ${NC})" choice
    [[ "$choice" =~ ^[Yy]$ ]] && CHANGE_MIRROR=true
    
    read -p "$(echo -e ${YELLOW}是否升级系统软件包? [Y/n]: ${NC})" choice
    [[ "$choice" =~ ^[Nn]$ ]] && UPDATE_SYSTEM=false
    
    echo ""
}

# ============================================================================
# 显示系统信息
# ============================================================================
show_system_info() {
    print_banner "系统信息"
    
    echo -e "${CYAN}操作系统:${NC} $(cat /etc/os-release | grep PRETTY_NAME | cut -d'"' -f2)"
    echo -e "${CYAN}内核版本:${NC} $(uname -r)"
    echo -e "${CYAN}系统架构:${NC} $(uname -m)"
    echo -e "${CYAN}CPU型号:${NC} $(grep 'model name' /proc/cpuinfo | head -1 | cut -d':' -f2 | xargs)"
    echo -e "${CYAN}CPU核心:${NC} $(nproc) 核"
    echo -e "${CYAN}总内存:${NC} $(free -h | grep Mem | awk '{print $2}')"
    echo -e "${CYAN}可用内存:${NC} $(free -h | grep Mem | awk '{print $7}')"
    echo -e "${CYAN}磁盘使用:${NC} $(df -h / | tail -1 | awk '{print $3 "/" $2 " (" $5 ")"}')"
    echo ""
}

# ============================================================================
# 显示安装摘要
# ============================================================================
show_summary() {
    print_banner "安装完成摘要"
    
    echo -e "${GREEN}${BOLD}✓ 安装成功！${NC}\n"
    
    if [ "$INSTALL_GIT" = true ] && command -v git &> /dev/null; then
        echo -e "${CYAN}Git:${NC} $(git --version)"
    fi
    
    if [ "$INSTALL_DOCKER" = true ] && command -v docker &> /dev/null; then
        echo -e "${CYAN}Docker:${NC} $(docker --version)"
    fi
    
    if [ "$INSTALL_PYTHON" = true ] && command -v python3 &> /dev/null; then
        echo -e "${CYAN}Python:${NC} $(python3 --version)"
        if command -v pip3 &> /dev/null; then
            echo -e "${CYAN}pip:${NC} $(pip3 --version)"
        fi
    fi
    
    if [ "$INSTALL_CURL" = true ] && command -v curl &> /dev/null; then
        echo -e "${CYAN}curl:${NC} $(curl --version | head -n 1)"
    fi
    
    echo ""
    echo -e "${YELLOW}${BOLD}重要提示:${NC}"
    
    if [ "$CONFIG_SSH" = true ]; then
        echo -e "  ${RED}•${NC} SSH密码登录已禁用，请确保已配置SSH密钥"
    fi
    
    if [ "$INSTALL_DOCKER" = true ]; then
        echo -e "  ${RED}•${NC} 非root用户使用Docker需执行: ${CYAN}sudo usermod -aG docker \$USER${NC}"
    fi
    
    echo -e "  ${RED}•${NC} 日志文件: ${CYAN}$LOG_FILE${NC}"
    echo -e "  ${RED}•${NC} 备份目录: ${CYAN}$BACKUP_DIR${NC}"
    
    echo ""
}

# ============================================================================
# 主函数
# ============================================================================
main() {
    # 清屏
    clear
    
    # 显示标题
    print_banner "基础环境一键部署脚本 v2.0"
    
    # 检查root权限
    check_root
    
    # 创建备份目录
    create_backup_dir
    
    # 检测系统
    detect_distro
    
    # 显示系统信息
    show_system_info
    
    # 解析命令行参数
    while [[ $# -gt 0 ]]; do
        case $1 in
            --no-interactive)
                INTERACTIVE_MODE=false
                shift
                ;;
            --mirror)
                CHANGE_MIRROR=true
                shift
                ;;
            *)
                shift
                ;;
        esac
    done
    
    # 交互式菜单
    if [ "$INTERACTIVE_MODE" = true ]; then
        interactive_menu
    fi
    
    echo ""
    log "开始安装，请稍候..."
    echo ""
    
    # 计算总步骤数
    local total_steps=0
    local current_step=0
    
    [ "$CHANGE_MIRROR" = true ] && ((total_steps++))
    ((total_steps++))  # 更新系统
    [ "$INSTALL_GIT" = true ] && ((total_steps++))
    [ "$INSTALL_DOCKER" = true ] && ((total_steps++))
    [ "$INSTALL_PYTHON" = true ] && ((total_steps++))
    [ "$INSTALL_CURL" = true ] && ((total_steps++))
    [ "$CONFIG_SSH" = true ] && ((total_steps++))
    [ "$UPDATE_SYSTEM" = true ] && ((total_steps++))
    
    # 执行安装
    if [ "$CHANGE_MIRROR" = true ]; then
        change_mirror
        ((current_step++))
        progress_bar $current_step $total_steps
        echo ""
    fi
    
    update_system
    ((current_step++))
    progress_bar $current_step $total_steps
    echo ""
    
    if [ "$INSTALL_GIT" = true ]; then
        install_git
        ((current_step++))
        progress_bar $current_step $total_steps
        echo ""
    fi
    
    if [ "$INSTALL_DOCKER" = true ]; then
        install_docker
        ((current_step++))
        progress_bar $current_step $total_steps
        echo ""
    fi
    
    if [ "$INSTALL_PYTHON" = true ]; then
        install_python
        ((current_step++))
        progress_bar $current_step $total_steps
        echo ""
    fi
    
    if [ "$INSTALL_CURL" = true ]; then
        install_curl
        ((current_step++))
        progress_bar $current_step $total_steps
        echo ""
    fi
    
    if [ "$CONFIG_SSH" = true ]; then
        configure_ssh
        ((current_step++))
        progress_bar $current_step $total_steps
        echo ""
    fi
    
    if [ "$UPDATE_SYSTEM" = true ]; then
        upgrade_system
        ((current_step++))
        progress_bar $current_step $total_steps
        echo ""
    fi
    
    echo ""
    
    # 显示摘要
    show_summary
    
    log_success "所有任务完成！"
}

# ============================================================================
# 脚本入口
# ============================================================================
main "$@"
