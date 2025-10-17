"use client";

import { useState } from "react";
import { useRouter } from "next/navigation";
import { createClient } from "@/lib/supabase/client";

export default function LogoutButton() {
  const [loading, setLoading] = useState(false);
  const router = useRouter();

  const handleLogout = async () => {
    setLoading(true);
    const supabase = createClient();
    try {
      await supabase.auth.signOut();
      // Go back to login; middleware will treat you as signed-out
      router.push("/login");
      router.refresh();
    } catch (e) {
      // (optional) surface an error toast here
      console.error(e);
    } finally {
      setLoading(false);
    }
  };

  return (
    <button
      onClick={handleLogout}
      disabled={loading}
      className="px-4 py-2 rounded-md font-medium text-white
                 bg-brand-600 hover:bg-brand-700 disabled:opacity-50"
      aria-label="Logout"
    >
      {loading ? "Signing out..." : "Logout"}
    </button>
  );
}
