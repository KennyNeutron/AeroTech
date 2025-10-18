// supabase/functions/ingest-reading/index.ts
import { serve } from "https://deno.land/std@0.192.0/http/server.ts";
import { createClient } from "https://esm.sh/@supabase/supabase-js@2";

// Secure service client (can bypass RLS safely)
const SUPABASE_URL = Deno.env.get("SUPABASE_URL")!;
const SERVICE_ROLE_KEY = Deno.env.get("SUPABASE_SERVICE_ROLE_KEY")!;
const supabase = createClient(SUPABASE_URL, SERVICE_ROLE_KEY);

serve(async (req) => {
  try {
    const auth = req.headers.get("authorization") || "";
    const token = auth.replace("Bearer ", "").trim();

    const body = await req.json();
    const { device_id, ph, tds, temp_c, water_level_code } = body;

    if (!device_id || !token) {
      return new Response("Missing device_id or token", { status: 400 });
    }

    // Verify device & secret
    const { data: device, error: devErr } = await supabase
      .from("devices")
      .select("id, secret")
      .eq("id", device_id)
      .maybeSingle();

    if (devErr || !device || device.secret !== token) {
      return new Response("Unauthorized device", { status: 401 });
    }

    // Insert new reading
    const { error: insertErr } = await supabase.from("sensor_readings").insert({
      device_id,
      recorded_at: new Date().toISOString(),
      ph,
      tds,
      temp_c,
      water_level_code,
      inserted_by: null, // optional
    });

    if (insertErr) throw insertErr;
    return new Response("OK", { status: 200 });
  } catch (err) {
    console.error(err);
    return new Response(String(err), { status: 500 });
  }
});
