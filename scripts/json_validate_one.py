import json,sys
p='features/feature_registry.json'
try:
    with open(p,'r', encoding='utf-8') as f:
        data = json.load(f)
    print('OK: parsed JSON. Total features =', len(data.get('features', {})))
    # Re-dump to detect other errors
    s = json.dumps(data, indent=2)
    print('Re-dump length:', len(s))
except Exception as e:
    print('ERROR:', type(e).__name__, e)
    import traceback
    traceback.print_exc()
    sys.exit(1)
