if (sku) {
    value = 0;
    for (var key in lnrr._current_report.skus_by_id) {
        var el = lnrr._current_report.skus_by_id[key];
        if ((el.sku.category_id == sku.category.id || Data.categories_by_id[el.sku.category_id].parents.indexOf(sku.category.id) + 1) && el._report.faces && el._report.amount) value += el._report.faces * 1 || 0;
    }
}