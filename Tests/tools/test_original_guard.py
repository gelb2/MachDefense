import gzip, hashlib, io, os, subprocess, sys, tempfile, unittest, zipfile
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[2]/'tools'))
import original_guard as guard


def archive(files):
    b=io.BytesIO()
    with zipfile.ZipFile(b,'w') as z:
        for name,data in files.items():z.writestr(name,data)
    return b.getvalue()


class GuardTests(unittest.TestCase):
    def blocked(self,name,data,known=set()):
        with self.assertRaises(ValueError):guard.inspect(name,data,known)

    def test_normal_source_and_asset_archive(self):
        guard.inspect('Sources/Core/a.cpp',b'int f(){return 0;}',set())
        guard.inspect('assets.zip',archive({'images/a.png':b'not an original'}),set())

    def test_ipa_case_and_renaming(self):
        self.blocked('game.IPA',b'x')
        self.blocked('a.png',archive({'Payload/Test.app/Info.plist':b'x'}))
        self.blocked('x.bin',b'original',{hashlib.sha256(b'original').hexdigest()})

    def test_nested_package_and_lfs(self):
        self.blocked('bundle.zip',archive({'renamed.bin':archive({'Payload/Game.app/Game':b'x'})}))
        self.blocked('renamed.dat',b'version https://git-lfs.github.com/spec/v1\noid sha256:abc\nsize 1\n')

    def test_wrapped_compression(self):
        self.blocked('disguised.bin',gzip.compress(archive({'Payload/Game.app/Game':b'x'})))
        self.blocked('data.bin',b'7z\xbc\xaf\x27\x1c'+b'data')

    def test_corrupt_archive(self):
        self.blocked('x.bin',b'PK\x03\x04broken')

    def test_symlink_stage_and_deleted_history(self):
        previous=guard.ROOT
        with tempfile.TemporaryDirectory() as tmp:
            guard.ROOT=Path(tmp)
            def run(*args):return subprocess.run(['git',*args],cwd=tmp,check=True,stdout=subprocess.PIPE,stderr=subprocess.PIPE)
            try:
                run('init','-b','main')
                run('config','user.name','Fixture')
                run('config','user.email','fixture@example.invalid')
                run('config','commit.gpgsign','false')
                p=Path(tmp)/'hidden.bin';p.write_bytes(archive({'Payload/Test.app/Info.plist':b'x'}))
                run('add','hidden.bin')
                with self.assertRaises(ValueError):
                    for name,oid in guard.staged():guard.inspect(name,guard.git('cat-file','blob',oid),set())
                run('commit','-m','fixture')
                run('rm','hidden.bin');run('commit','-m','remove')
                with self.assertRaises(ValueError):
                    for name,oid in guard.history():guard.inspect(name,guard.git('cat-file','blob',oid),set())
                (Path(tmp)/'link').symlink_to('/outside/original')
                run('add','link')
                with self.assertRaises(ValueError):list(guard.staged())
            finally:guard.ROOT=previous


if __name__=='__main__':unittest.main()
