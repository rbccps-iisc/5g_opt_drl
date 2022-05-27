ENV_NAME=${1:-ray-de}
PYTHON_VER=${2:-3.8}
CONDA_DIR=${3:-$HOME/miniconda}
CONDA_URL=${4:-https://repo.anaconda.com/miniconda/Miniconda3-py38_4.11.0-Linux-x86_64.sh}

<<install_check
    Check install using conda --version.
    If it is not installed it will return empty string.
    Install conda.
install_check
conda_installed=$(conda --version)
if [[ ! $conda_installed ]]
then
    wget -O miniconda.sh $CONDA_URL
    /bin/bash miniconda.sh -b -p "$CONDA_DIR"
    rm miniconda.sh
    echo "PATH=$CONDA_DIR/bin:$PATH" >> ~/.bashrc
    source ~/.bashrc
    conda init
    conda config --set auto_activate_base false
else
    echo "conda already installed and available."
fi


<<env_check
    Identify env dir using grep.
    First grep extracts the line having env name.
    second grep extracts the complete path of the env with regex:
        -o / --only-matching - only outputs the matched part
        [^ *] - ^ represents the compliment of any number of spaces 
                i.e. "<space>*", where spaces are delimiters
        * - The outer * in [^ *]* matches preceding pattern [^ *]
            zero or more number of times
        $ - matches the end of string and facilitates picking the 
            last entry
    The above regex is required as the conda env lists can have outputs:
        ray-de                   /root/miniconda/envs/ray-de
        ray-de                *  /root/miniconda/envs/ray-de
env_check
if [[ $CONDA_DEFAULT_ENV == $ENV_NAME ]]
then
    echo "${ENV_NAME} is already active."
else
    env_dir=$(conda env list | grep "envs/${ENV_NAME}" | grep -o "[^ *]*$")
    if [[ ! -d $env_dir ]]
    then
        conda create --name $ENV_NAME python=$PYTHON_VER -y
        conda activate $ENV_NAME
        # echo "conda activate ${ENV_NAME}" >> ~/.bashrc
        # source ~/.bashrc
    else
        conda activate $ENV_NAME
        echo "${ENV_NAME} conda env already exists. Activated it."
    fi
fi