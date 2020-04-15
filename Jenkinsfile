def version='2019.2'

node('merlin') {
    if (!fileExists("/fpga_tools/Xilinx/kernel/imsar-v$version")) {
        dir("/fpga_tools/Xilinx/kernel") {
            dir("linux-xlnx")
            {
                sh("git fetch origin")
            }        
            sh("git clone --reference linux-xlnx/ --branch imsar-v$version https://github.com/imsarllc/linux-xlnx.git imsar-v$version")
        }
    }
    dir("/fpga_tools/Xilinx/kernel/imsar-v$version") {
        stage('Update') {
            sh('git pull')
        }
        def env="source /fpga_tools/Xilinx/Vivado/$version/settings64.sh;"
        stage('Build Kernel') {
            sh("$env make xilinx_zynq_defconfig && make -j8 uImage UIMAGE_LOADADDR=0x8000")
            currentBuild.description = sh(script: 'cat include/config/kernel.release', returnStdout: true)
        }
        stage('Build Modules') {
            sh("$env make modules -j8 && make modules_install INSTALL_MOD_PATH=usr/")
        }
        stage('Artifacts') {
            sh('cp arch/arm/boot/uImage .')
            archiveArtifacts artifacts: 'System.map,uImage', fingerprint: true
        }
    }
}
