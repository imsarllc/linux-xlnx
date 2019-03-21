def version='2016.4'

node('merlin') {
    dir("/fpga_tools/Xilinx/kernel/imsar-v$version") {
        stage('Update') {
            sh('git pull')
        }
        def env="source /fpga_tools/Xilinx/Vivado/$version/settings64.sh; export ARCH=arm;"
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
