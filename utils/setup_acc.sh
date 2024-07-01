#/usr/bin/env bash

# Set the variable 'kernel_dir' to be the location where the Jupyter kernel will be stored.
kernel_dir=${HOME}/.local/share/jupyter/kernels/gpu_directives

# 'curdir' is the current directory where the script is running.
curdir=$(dirname $(readlink -f "$0"))

# 'rootdir' is the parent directory of the current script.
rootdir=$(dirname $curdir)

# 'conf_dir' is the directory where the configuration files for the kernel will be stored.
conf_dir=${rootdir}/utils

# 'ipython_dir' is the directory where the IPython profile will be located.
ipython_dir=${HOME}/.ipython/profile_default


# Setup jupyterhub
mkdir -p $kernel_dir/
cat > ${kernel_dir}/kernel.sh <<EOF
#!/bin/bash

module purge # disable the external environment

# Activate your Python virtual environment
module load anaconda-py3/2023.03
conda activate gpu_directives

# set the IDR_CONFIG_FILE environment variable to point to your jean-zay.json file, which contains the configuration for Jean Zay.
export IDR_CONFIG_FILE=${conf_dir}/jean-zay.json

exec python -m ipykernel \$@
EOF
chmod +x ${kernel_dir}/kernel.sh

cat > ${kernel_dir}/kernel.json <<EOF
{
"argv": [
 "${kernel_dir}/kernel.sh",
 "-f",
 "{connection_file}"
],
"display_name": "GPU Directives",
"language": "python",
"metadata": {
"debugger": true
 }
}
EOF

# Setup ipython
mkdir -p ${ipython_dir}
cat > ${ipython_dir}/ipython_config.py <<EOF
c.InteractiveShellApp.extensions = ["idrcomp"]
EOF

if [[ -z ${CONDA_PREFIX} ]]
then
    module load anaconda-py3/2023.03
    conda activate gpu_directives
fi
cd ${rootdir}/idrcomp && pip install -e .
