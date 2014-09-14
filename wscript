from waflib.Build import BuildContext


class run(BuildContext):
    'runs system on emulator or virtual machine'
    cmd = 'run'
    fun = 'run'


def options(opt):
    opt.recurse('src')


def configure(cnf):
    cnf.recurse('src')


def build(bld):
    bld.recurse('src')


def run(run):
    build(run)
    run.recurse('src')
