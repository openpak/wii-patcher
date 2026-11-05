import struct, sys
class SO:
    def __init__(s, path):
        d=open(path,'rb').read(); s.d=d
        phoff=struct.unpack('>I',d[0x1c:0x20])[0]; n=struct.unpack('>H',d[0x2c:0x2e])[0]
        s.segs=[]
        for i in range(n):
            p=struct.unpack('>8I',d[phoff+i*32:phoff+i*32+32])
            if p[0]==1 and p[2]>=0x10000000: s.segs.append(dict(off=p[1],va=p[2],fsz=p[4],msz=p[5],flg=p[6]))
        s.rx=[g for g in s.segs if g['flg']&1][0]
    def rd(s,va,n):
        for g in s.segs:
            if g['va']<=va<g['va']+g['fsz']: o=g['off']+va-g['va']; return s.d[o:o+n]
        return None
    def w(s,va): return struct.unpack('>I',s.rd(va,4))[0]
    def h(s,va): return struct.unpack('>H',s.rd(va,2))[0]
    def find_str(s,txt):
        b=txt.encode()+b'\0'; o=s.d.find(b)
        for g in s.segs:
            if g['off']<=o<g['off']+g['fsz']: return g['va']+o-g['off']
    def lit_refs(s,val):
        """thumb LDR Rd,[PC,#imm8*4] sites whose literal == val"""
        rx=s.rx; out=[]
        for va in range(rx['va'],rx['va']+rx['fsz'],2):
            hw=s.h(va)
            if hw>>11==0b01001:
                lit=((va+4)&~3)+(hw&0xff)*4
                if rx['va']<=lit<rx['va']+rx['fsz']-3 and s.w(lit)==val: out.append((va,(hw>>8)&7))
        return out
    def bl_targets(s):
        """thumb BL pairs: F000-F7FF prefix then F800-FFFF suffix (BL) or E800 (BLX)"""
        rx=s.rx; t={}
        for va in range(rx['va'],rx['va']+rx['fsz']-2,2):
            a=s.h(va); b=s.h(va+2)
            if a>>11==0b11110 and (b>>11) in (0b11111,0b11101):
                off=((a&0x7ff)<<12)|((b&0x7ff)<<1)
                if off&0x400000: off-=0x800000
                tgt=va+4+off
                if b>>11==0b11101: tgt&=~3
                t.setdefault(tgt,[]).append(va)
        return t
