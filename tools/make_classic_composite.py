"""Wrap an existing BTBP unchanged as an independent furniture composition.

This writes new project files only. It neither places objects nor changes saves.
"""
from pathlib import Path
import argparse
import hashlib
import json
import math
import struct


def inspect(data):
    if len(data)<40 or data[:4]!=b'BTBP' or struct.unpack_from('<HH',data,4)!=(1,0):
        raise ValueError('Unsupported BTBP header')
    step=list(struct.unpack_from('<3f',data,8));dims=list(struct.unpack_from('<3H',data,20))
    if any(not math.isfinite(v) or v<.5 or v>4 or v*2!=round(v*2) for v in step):raise ValueError('Unsupported source grid')
    total=math.prod(dims)
    if not 1<=total<=65536:raise ValueError('Source exceeds classic blueprint cell budget')
    nm=struct.unpack_from('<H',data,30)[0]
    if nm>64:raise ValueError('Invalid source palette')
    at=32+16*nm
    if at+4>len(data):raise ValueError('Truncated palette')
    runs=struct.unpack_from('<I',data,at)[0];at+=4;cells=[]
    for _ in range(runs):
        if at+4>len(data):raise ValueError('Truncated cell runs')
        count,material,padding=struct.unpack_from('<HBB',data,at);at+=4
        if not count or padding or (material!=255 and material>=nm) or len(cells)+count>total:raise ValueError('Invalid cell run')
        cells.extend([material]*count)
    if len(cells)!=total or at+4>len(data):raise ValueError('Incomplete source cells')
    count=struct.unpack_from('<I',data,at)[0];at+=4
    if not 1<=count<=512 or at+count*32!=len(data):raise ValueError('Expected complete native object records')
    objects=[]
    for i in range(count):
        row=struct.unpack_from('<I7f',data,at+32*i)
        if not row[0] or any(not math.isfinite(v) or abs(v)>65536 for v in row[1:4]) or any(not math.isfinite(v) or abs(v)>1.001 for v in row[4:]) or not .99<=sum(v*v for v in row[4:])<=1.01:raise ValueError('Invalid native object transform')
        objects.append({'index':i,'item':row[0],'hex':f'0x{row[0]:08x}','position':list(row[1:4]),'rotation':list(row[4:]),'recordOffset':at+32*i})
    return {'sourceSHA256':hashlib.sha256(data).hexdigest(),'sourceBytes':len(data),'step':step,'dimensions':dims,
            'sourceVoxelCells':sum(c!=255 for c in cells),'objectCount':count,'objects':objects,
            'serializedProperties':['inventoryID','position','quaternion'],
            'notSerialized':['templateGUID','scale','containerContents','fuel','logicLinks','customState'],
            'compositePlacementValidatedInGame':False,'persistenceValidated':False}


def generate(source,out,prefix='Classic_Composition',anchor=(0,0,0),instances=None):
    data=source.read_bytes();analysis=inspect(data)
    if out.exists():raise ValueError('Output exists; no overwrite')
    if not prefix or any(c not in 'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_' for c in prefix):raise ValueError('Use a safe output prefix')
    anchor=list(anchor);instances=instances or [[0,0,0,0]]
    def grid(v):return type(v) in (int,float) and math.isfinite(v) and abs(v)<=65536 and v*2==round(v*2)
    if len(anchor)!=3 or not all(grid(v) for v in anchor) or not 1<=len(instances)<=128:raise ValueError('Invalid anchor/instance count')
    modules=[]
    for row in instances:
        if len(row)!=4 or not all(grid(v) for v in row[:3]) or row[3] not in (0,90,180,270):raise ValueError('Invalid module transform')
        modules.append({'file':prefix+'_Source.btbp','offset':row[:3],'rotation':int(row[3]),'category':'furniture',
                        'content':'objects','objectPlacement':'blueprint'})
    manifest={'format':'BuildingToolsExtendedProject','version':1,'name':prefix+' - composition classique conservee',
              'gridSize':.5,'anchor':anchor,'modules':modules,'pieces':[]}
    out.mkdir(parents=True)
    (out/(prefix+'_Source.btbp')).write_bytes(data)
    (out/(prefix+'.btproject')).write_bytes((json.dumps(manifest,ensure_ascii=False,indent=2)+'\n').encode())
    analysis.update({'sourceName':source.name,'moduleFile':prefix+'_Source.btbp','buildVoxelCells':0,
                     'sourceVoxelRole':'captured support contacts, never rebuilt','instances':instances,'anchor':anchor})
    (out/(prefix+'_Analysis.json')).write_bytes((json.dumps(analysis,ensure_ascii=False,indent=2)+'\n').encode())
    return out/(prefix+'.btproject')


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--blueprint',type=Path,required=True);p.add_argument('--outdir',type=Path,required=True)
    p.add_argument('--prefix',default='Classic_Composition');p.add_argument('--anchor',nargs=3,type=float,default=[0,0,0])
    p.add_argument('--instance',nargs=4,type=float,action='append',help='X Y Z yaw; repeat for reusable furniture sets')
    a=p.parse_args()
    try:print(generate(a.blueprint,a.outdir,a.prefix,a.anchor,a.instance))
    except (ValueError,OSError,struct.error) as e:p.exit(1,f'Refused: {e}\n')


if __name__=='__main__':main()
