# kill all nodes
fuser -k outputnode*

# remove created folders (if any)
rm -rf _Blockchains
rm -rf _Blocks
rm -rf _Hashes
rm -rf _Pings
rm -rf _Sessions

# remove previous outputs
rm outputnode*
fuser -k *
